#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <csignal>
#include <sys/wait.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <cstring>
#include <system_error>
#include <sstream>

#include "Secure/Config/ServerConfig.hpp"
#include "Core/Cluster/Cluster.hpp"
#include "Core/Server/Server.hpp"
#include "Utils/Json.hpp"
#include "Core/Http/Middleware/Middleware.hpp"

int main()
{
    ServerConfig config;

    std::cout << "Server listening on port " << config.PORT << "...\n";

    Server server = Server(config);

    Middleware authMiddleware = Middleware([](Http::Request &req, Http::Response &res, auto next)
                                  {
        std::string token = req.getQuery("token");
        if (token != "123") {
            res << 401 << "Unauthorized";
            return;
        } 
        next(); 
    });

    server.Static("/static", "./public");

    server.Get("/", {}, [](const Http::Request &req, Http::Response &res)
               { res << 200 << "Home Page - Nerva HTTP Server"; });

    server.Get("/home", {}, [](const Http::Request &req, Http::Response &res)
               { res << 200 << "Welcome!"; });

    server.Get("/test/:id", {}, [](const Http::Request &req, Http::Response &res)
               { res << 200 << "Test ID: " << req.getParam("id"); });

    server.Post("/test", {}, [](const Http::Request &req, Http::Response &res)
                              {
        const std::string jsonResponse = R"({"message": "Test POST successful!"})";
        res << 200 << Json::ParseAndReturnBody(jsonResponse); 
    });

    server.Get("/image-test", {}, [](const Http::Request &req, Http::Response &res)
                                 { res.SendFile("./public/a.jpg"); });

    server["GET"].Use("/protected", {authMiddleware}, [](const Http::Request &req, Http::Response &res)
                      { 
                const std::string jsonResponse = R"({"message": "Protected area - Welcome!"})";
                res << 200 << Json::ParseAndReturnBody(jsonResponse); 
    });

    server["GET"].Use("/admin", {authMiddleware}, [](const Http::Request &req, Http::Response &res)
                      { 
                const std::string jsonResponse = R"({"message": "Admin panel", "status": "active"})";
                res << 200 << Json::ParseAndReturnBody(jsonResponse); 
    });

    server["GET"].Use("/redirect", {authMiddleware}, [](const Http::Request &req, Http::Response &res)
                      { res.MovedRedirect("/home"); });

    server["GET"].Register("/register-test").Use(authMiddleware).Then([](const Http::Request &req, Http::Response &res)
                                                            {
        const std::string jsonResponse = R"({"message": "Register test successful!"})";
        res << 200 << Json::ParseAndReturnBody(jsonResponse); 
    });

    server.Get("/secure")
        .Use(authMiddleware)
        .Then([](const Http::Request &req, Http::Response &res)
              {
        const std::string jsonResponse = R"({"message": "Secure area", "access": "granted"})";
        res << 200 << Json::ParseAndReturnBody(jsonResponse); 
    });

    Router apiRouter;
    apiRouter.Get("/users", {}, [](const Http::Request &req, Http::Response &res)
                  { res << 200 << "User list"; });
    
    apiRouter.Get("/users/:id", {}, [](const Http::Request &req, Http::Response &res)
                  { res << 200 << "User ID: " << req.getParam("id"); });

    server.Use("/api", apiRouter);

    server.Group("/api/v1").Then([](Router &r) {
        r.Get("/users").Then([](const Http::Request &req, Http::Response &res)
                            { res << 200 << "API v1 - Users"; });
        
        r.Get("/posts").Then([](const Http::Request &req, Http::Response &res)
                            { res << 200 << "API v1 - Posts"; });
    });

    server.Group("/admin").Then([](Router &r) {
        r.Get("/dashboard").Then([](const Http::Request &req, Http::Response &res)
                                { res << 200 << "Admin Dashboard"; });
        
        r.Get("/settings").Then([](const Http::Request &req, Http::Response &res)
                               { res << 200 << "Admin Settings"; });
    });

    server.Group("/blog").Then([](Router &r) {
        r.Get("/posts").Then([](const Http::Request &req, Http::Response &res)
                            { res << 200 << "Blog Posts"; });
        
        r.Get("/posts/:id").Then([](const Http::Request &req, Http::Response &res)
                                { res << 200 << "Blog post ID: " << req.getParam("id"); });
        
        r.Get("/categories").Then([](const Http::Request &req, Http::Response &res)
                                 { res << 200 << "Blog Categories"; });
    });

    server.Get("/*").Then([](const Http::Request &req, Http::Response &res)
                          { res << 404 << "Page not found - 404"; });

    server.Start();
    server.Stop();
    return 0;
}