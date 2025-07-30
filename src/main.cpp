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

    std::cout << "Sunucu " << config.PORT << " portunda dinleniyor...\n";

    Server server = Server(config);

    Middleware login = Middleware([](Http::Request &req, Http::Response &res, auto next)
                                  {
        std::string token = req.getQuery("token");
        if (token != "123") {
            res << 401 << "Unauthorized";
            return;
        } 

        next(); });

    server.Static("/static", "./public");

    Router router;
    router.Get("/test/:id", {}, [](const Http::Request &req, Http::Response &res)
               { res << 200 << "Test ID: " << req.getParam("id"); });

    server.Get("/", {}, [](const Http::Request &req, Http::Response &res)
               { res << 200 << "Hello, World!"; });

    server.Get("/a", {}, [](const Http::Request &req, Http::Response &res)
               { res << 200 << "Hello, A!"; });

    server["GET"].Use("/b", {login}, [](const Http::Request &req, Http::Response &res)
                      { 
                const std::string jsonResponse = R"({"message": "Hello, B!"})";
                res << 200 << Json::ParseAndReturnBody(jsonResponse); });

    server["GET"].Register("/registerTest").Use(login).Then([](const Http::Request &req, Http::Response &res)
                                                            {
        const std::string jsonResponse = R"({"message": "Hello, Register!"})";
        res << 200 << Json::ParseAndReturnBody(jsonResponse); });

    server.Get("/123")
        .Use(login)
        .Then([](const Http::Request &req, Http::Response &res)
              {
        const std::string jsonResponse = R"({"message": "Hello, 123!"})";
        res << 200 << Json::ParseAndReturnBody(jsonResponse); });

    server.Post("/test").Then([](const Http::Request &req, Http::Response &res)
                              {
        const std::string jsonResponse = R"({"message": "Hello, 123!"})";
        res << 200 << Json::ParseAndReturnBody(jsonResponse); });

    server.Use("/asd", login);
    server.Use("/asd", router);

    server.Start();

    server.Stop();
    return 0;
}