// Nerva HTTP Server - Demo Application
// A clean and well-documented main file demonstrating all features
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

#include "Server.hpp"
#include "Middleware.hpp"
#include "Json.hpp"
#include "ViewEngine/NervaEngine.hpp"

int main() {
    // Server and configuration
    Server server;
    server.SetConfigFile("server");
    std::cout << "Server listening on port " << 8080 << "...\n";

    // Static file serving
    server.Static("/static", "./public");

    // Template engine integration
    Nerva::Engine *engine = new Nerva::Engine();
    engine->setViewsDirectory("./views");
    server.Set("view engine", engine);

    // Basic route
    server.Get("/", {}, [](const Http::Request &req, Http::Response &res) {
        res << 200 << "Home Page - Nerva HTTP Server";
    });

    // Dynamic parameter route
    server.Get("/test/:id", {}, [](const Http::Request &req, Http::Response &res) {
        res << 200 << "Test ID: " << req.getParam("id");
    });

    // File upload (multipart/form-data) example
    server.Post("/upload", {}, [](const Http::Request &req, Http::Response &res) {
        auto fileData = req.getFormData("file");
        if (fileData.isFile && !fileData.file.empty()) {
            fileData.file.save("./public/" + fileData.filename);
            res << 200 << "File uploaded successfully: " << fileData.filename;
        } else {
            res << 400 << "File upload failed.";
        }
    });

    // JSON response example
    server.Post("/json", {}, [](const Http::Request &req, Http::Response &res) {
        const std::string jsonResponse = R"({"message": "JSON POST successful!"})";
        res << 200 << Json::ParseAndReturnBody(jsonResponse);
    });

    // Direct static file serving
    server.Get("/image-test", {}, [](const Http::Request &req, Http::Response &res) {
        res.SendFile("./public/a.jpg");
    });

    // Middleware-protected route
    Middleware authMiddleware = Middleware([](Http::Request &req, Http::Response &res, auto next) {
        std::string token = req.getQuery("token");
        if (token != "123") {
            res << 401 << "Unauthorized";
            return;
        }
        next();
    });

    server["GET"].Use("/protected", {authMiddleware}, [](const Http::Request &req, Http::Response &res) {
        res << 200 << Json::ParseAndReturnBody(R"({"message": "Protected area - Welcome!"})");
    });

    // Redirect example
    server["GET"].Use("/redirect", {authMiddleware}, [](const Http::Request &req, Http::Response &res) {
        res.MovedRedirect("/home");
    });

    // Route chaining and group route examples
    server["GET"].Register("/register-test").Use(authMiddleware).Then([](const Http::Request &req, Http::Response &res) {
        res << 200 << Json::ParseAndReturnBody(R"({"message": "Register test successful!"})");
    });

    server.Get("/secure")
        .Use(authMiddleware)
        .Then([](const Http::Request &req, Http::Response &res) {
            res << 200 << Json::ParseAndReturnBody(R"({"message": "Secure area", "access": "granted"})");
        });

    // API Router example
    Router apiRouter;
    apiRouter.Get("/users", {}, [](const Http::Request &req, Http::Response &res) {
        res << 200 << "User list";
    });
    apiRouter.Get("/users/:id", {}, [](const Http::Request &req, Http::Response &res) {
        res << 200 << "User ID: " << req.getParam("id");
    });
    server.Use("/api", apiRouter);

    // Group route (API v1)
    server.Group("/api/v1").Then([](Router &r) {
        r.Get("/users").Then([](const Http::Request &req, Http::Response &res) {
            res << 200 << "API v1 - Users";
        });
        r.Get("/posts").Then([](const Http::Request &req, Http::Response &res) {
            res << 200 << "API v1 - Posts";
        });
    });

    // Admin panel group route
    server.Group("/admin").Then([](Router &r) {
        r.Get("/dashboard").Then([](const Http::Request &req, Http::Response &res) {
            res << 200 << "Admin Dashboard";
        });
        r.Get("/settings").Then([](const Http::Request &req, Http::Response &res) {
            res << 200 << "Admin Settings";
        });
    });

    // Blog group route
    server.Group("/blog").Then([](Router &r) {
        r.Get("/posts").Then([](const Http::Request &req, Http::Response &res) {
            res << 200 << "Blog Posts";
        });
        r.Get("/posts/:id").Then([](const Http::Request &req, Http::Response &res) {
            res << 200 << "Blog post ID: " << req.getParam("id");
        });
        r.Get("/categories").Then([](const Http::Request &req, Http::Response &res) {
            res << 200 << "Blog Categories";
        });
    });

    // Dynamic page rendering with template engine
    server.Get("/products").Then([](const Http::Request &req, Http::Response &res) {
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
                {{"id", "101"}, {"name", "Smartphone"}, {"price", 7999.90}, {"inStock", true}},
                {{"id", "205"}, {"name", "Laptop"}, {"price", 12499.99}, {"inStock", false}},
                {{"id", "302"}, {"name", "Wireless Headphones"}, {"price", 1299.50}, {"inStock", true}}
            }},
            {"features", {"Fast Delivery", "Free Returns", "Original Product Guarantee"}}
        };
        res.Render("productPage", data);
    });

    // Custom 404 (catch-all)
    server.Get("/*").Then([](const Http::Request &req, Http::Response &res) {
        res.Render("notFound", nlohmann::json{});
    });

    // Start the server
    server.Start();
    server.Stop();
    return 0;
}