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

#include "Core/Cluster/Cluster.hpp"
#include "Core/Server/Server.hpp"
#include "Utils/Json.hpp"
#include "Core/Http/Middleware/Middleware.hpp"
#include "ViewEngine/Engine.hpp"

int main()
{
    Server server = Server();
    server.SetConfigFile("server");

    std::cout << "Server listening on port " << 8080 << "...\n";

    Middleware authMiddleware = Middleware([](Http::Request &req, Http::Response &res, auto next)
                                           {
        std::string token = req.getQuery("token");
        if (token != "123") {
            res << 401 << "Unauthorized";
            return;
        } 
        next(); });

    server.Static("/static", "./public");

    Nerva::Engine *engine = new Nerva::Engine();
    engine->setViewsDirectory("./views");

    server.Set("view engine", engine);

    server.Get("/", {}, [](const Http::Request &req, Http::Response &res)
               { res << 200 << "Home Page - Nerva HTTP Server"; });

    server.Get("/home", {}, [](const Http::Request &req, Http::Response &res)
               { res << 200 << "Welcome!"; });

    server.Get("/test/:id", {}, [](const Http::Request &req, Http::Response &res)
               { res << 200 << "Test ID: " << req.getParam("id"); });

    server.Post("/test", {}, [](const Http::Request &req, Http::Response &res)
                {
        const std::string jsonResponse = R"({"message": "Test POST successful!"})";
        res << 200 << Json::ParseAndReturnBody(jsonResponse); });

    server.Get("/image-test", {}, [](const Http::Request &req, Http::Response &res)
               { res.SendFile("./public/a.jpg"); });

    server["GET"].Use("/protected", {authMiddleware}, [](const Http::Request &req, Http::Response &res)
                      { 
                const std::string jsonResponse = R"({"message": "Protected area - Welcome!"})";
                res << 200 << Json::ParseAndReturnBody(jsonResponse); });

    server["GET"].Use("/admin", {authMiddleware}, [](const Http::Request &req, Http::Response &res)
                      { 
                const std::string jsonResponse = R"({"message": "Admin panel", "status": "active"})";
                res << 200 << Json::ParseAndReturnBody(jsonResponse); });

    server["GET"].Use("/redirect", {authMiddleware}, [](const Http::Request &req, Http::Response &res)
                      { res.MovedRedirect("/home"); });

    server["GET"].Register("/register-test").Use(authMiddleware).Then([](const Http::Request &req, Http::Response &res)
                                                                      {
        const std::string jsonResponse = R"({"message": "Register test successful!"})";
        res << 200 << Json::ParseAndReturnBody(jsonResponse); });

    server.Get("/secure")
        .Use(authMiddleware)
        .Then([](const Http::Request &req, Http::Response &res)
              {
        const std::string jsonResponse = R"({"message": "Secure area", "access": "granted"})";
        res << 200 << Json::ParseAndReturnBody(jsonResponse); });

    Router apiRouter;
    apiRouter.Get("/users", {}, [](const Http::Request &req, Http::Response &res)
                  { res << 200 << "User list"; });

    apiRouter.Get("/users/:id", {}, [](const Http::Request &req, Http::Response &res)
                  { res << 200 << "User ID: " << req.getParam("id"); });

    server.Use("/api", apiRouter);

    server.Group("/api/v1").Then([](Router &r)
                                 {
        r.Get("/users").Then([](const Http::Request &req, Http::Response &res)
                            { res << 200 << "API v1 - Users"; });
        
        r.Get("/posts").Then([](const Http::Request &req, Http::Response &res)
                            { res << 200 << "API v1 - Posts"; }); });

    server.Group("/admin").Then([](Router &r)
                                {
        r.Get("/dashboard").Then([](const Http::Request &req, Http::Response &res)
                                { res << 200 << "Admin Dashboard"; });
        
        r.Get("/settings").Then([](const Http::Request &req, Http::Response &res)
                               { res << 200 << "Admin Settings"; }); });

    server.Group("/blog").Then([](Router &r)
                               {
        r.Get("/posts").Then([](const Http::Request &req, Http::Response &res)
                            { res << 200 << "Blog Posts"; });
        
        r.Get("/posts/:id").Then([](const Http::Request &req, Http::Response &res)
                                { res << 200 << "Blog post ID: " << req.getParam("id"); });
        
        r.Get("/categories").Then([](const Http::Request &req, Http::Response &res)
                                 { res << 200 << "Blog Categories"; }); });

    server.Get("/products").Then([](const Http::Request &req, Http::Response &res)
                                 {
    nlohmann::json data = {
        {"pageTitle", "Super Products"},
        {"showPromo", true},
        {"promoMessage", "TODAY'S SPECIAL DISCOUNT!"},
        {"user", {
            {"name", "Ayşe Demir"},
            {"premium", true},
            {"cartItems", "3"}
        }},
        {"products", {
            {
                {"id", "101"},
                {"name", "Smartphone"},
                {"price", 7999.90},
                {"inStock", true}
            },
            {
                {"id", "205"},
                {"name", "Laptop"},
                {"price", 12499.99},
                {"inStock", false}
            },
            {
                {"id", "302"},
                {"name", "Wireless Headphones"},
                {"price", 1299.50},
                {"inStock", true}
            }
        }},
        {"features", {
            "Fast Delivery",
            "Free Returns",
            "Original Product Guarantee"
        }}
    };
    
    res.Render("productPage", data); });

    server.Get("/*").Then([](const Http::Request &req, Http::Response &res)
                          { res.Render("notFound", nlohmann::json{}); });

    server.Start();
    server.Stop();
    return 0;
}