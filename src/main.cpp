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

int main()
{
    ServerConfig config;

    std::cout << "Sunucu " << config.PORT << " portunda dinleniyor...\n";

    Server server = Server(config);

    server.Get("/", [](const Http::Request &req, Http::Response &res)
               {
            res.setStatus(200, "OK");
            res.setHeader("Content-Type", "text/plain");
            res.body = "Hello, World!"; });

    server.Get("/a", [](const Http::Request &req, Http::Response &res)
               { 
            res.setHeader("Content-Type", "application/json");
            const std::string jsonResponse = R"({"message": "Hello, A!"})";
            res.body = Json::ParseAndReturnBody(jsonResponse); });

    server.Start();

    server.Stop();
    return 0;
}