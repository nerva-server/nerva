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
#include <map>

#include "Server.hpp"
#include "Middleware.hpp"
#include "Json.hpp"
#include "ViewEngine/NervaEngine.hpp"

#include "RateLimiter.hpp"
#include "Cors.hpp"

std::map<std::string, std::string> users = {
    {"admin", "password123"},
    {"user1", "password456"},
    {"demo", "demo123"}};

std::map<std::string, std::string> sessions;

int main()
{
    Server server;
    server.SetConfigFile("server");
    std::cout << "Server listening on port " << 8080 << "...\n";

    server.Static("/static", "./public");

    RateLimiter rateLimiter;

    rateLimiter.Config.windowMs = 60 * 1000;
    rateLimiter.Config.limit = 100;

    server.Use("/*", rateLimiter);

    CorsConfig config;
    Cors cors = Cors(config);

    cors.setPolicy(CorsPolicy::BLOCK_ALL);

    server.Use("/*", cors);

    Nerva::Engine *engine = new Nerva::Engine();
    engine->setViewsDirectory("./views");
    server.Set("view engine", engine);

    server.Get("/", {}, [](const Http::Request &req, Http::Response &res, auto next)
               {
        auto sessionId = res.getCookie("session_id");
        if (sessionId && sessions.find(*sessionId) != sessions.end()) {
            res.TemporaryRedirect("/dashboard");
        } else {
            res.TemporaryRedirect("/login");
        } });

    server.Get("/login", {}, [](const Http::Request &req, Http::Response &res, auto next)
               {
        nlohmann::json data = {
            {"pageTitle", "Login - Nerva HTTP Server"},
            {"error", ""}
        };
        res.Render("login", data); });

    server.Post("/login", {}, [](const Http::Request &req, Http::Response &res, auto next)
                {
        std::string username = req.getFormData("username").value;
        std::string password = req.getFormData("password").value;

        if (users.find(username) != users.end() && users[username] == password) {
            std::string sessionId = "sess_" + std::to_string(std::time(nullptr)) + "_" + username;
            sessions[sessionId] = username;
            
            Http::CookieOptions cookieOpts;
            cookieOpts.maxAge = std::chrono::hours(24);
            cookieOpts.httpOnly = true;
            cookieOpts.secure = false;
            cookieOpts.sameSite = "Lax";
            
            res.setCookie("session_id", sessionId, cookieOpts);
            res.TemporaryRedirect("/dashboard");
        } else {
            nlohmann::json data = {
                {"pageTitle", "Login - Nerva HTTP Server"},
                {"error", "Invalid username or password"}
            };
            res.Render("login", data);
        } });

    server.Get("/dashboard", {}, [](const Http::Request &req, Http::Response &res, auto next)
               {
        auto sessionId = res.getCookie("session_id");
        if (!sessionId || sessions.find(*sessionId) == sessions.end()) {
            res.TemporaryRedirect("/login");
            return;
        }
        
        std::string username = sessions[*sessionId];
        nlohmann::json data = {
            {"pageTitle", "Dashboard - Nerva HTTP Server"},
            {"username", username},
            {"sessionId", *sessionId},
            {"loginTime", std::to_string(std::time(nullptr))}
        };
        res.Render("dashboard", data); });

    server.Get("/logout", {}, [](const Http::Request &req, Http::Response &res, auto next)
               {
        auto sessionId = res.getCookie("session_id");
        if (sessionId) {
            sessions.erase(*sessionId);
            res.removeCookie("session_id");
        }
        res.TemporaryRedirect("/login"); });

    server.Get("/cookies", {}, [](const Http::Request &req, Http::Response &res, auto next)
               {
        Http::CookieOptions basicOpts;
        basicOpts.maxAge = std::chrono::hours(1);
        res.setCookie("basic_cookie", "Hello World", basicOpts);
        
        Http::CookieOptions secureOpts;
        secureOpts.maxAge = std::chrono::hours(2);
        secureOpts.httpOnly = true;
        secureOpts.secure = false; 
        res.setSignedCookie("secure_cookie", "Secret Data", "my-secret-key", secureOpts);
        
        Http::CookieOptions sessionOpts;
        sessionOpts.maxAge = std::chrono::hours(24);
        sessionOpts.httpOnly = true;
        sessionOpts.sameSite = "Strict";
        res.setCookie("session_cookie", "User Session Data", sessionOpts);
        
        std::string basicValue = res.getCookieValue("basic_cookie", "Not Set");
        auto secureValue = res.getSignedCookie("secure_cookie", "my-secret-key");
        
        nlohmann::json data = {
            {"pageTitle", "Cookie Examples"},
            {"basicCookie", basicValue},
            {"secureCookie", secureValue.value_or("Invalid or Not Set")},
            {"allCookies", nlohmann::json::object()}
        };
        
        for (const auto& [name, value] : res.incomingCookies) {
            data["allCookies"][name] = value;
        }
        
        res.Render("cookies", data); });

    server.Get("/cookie-manager", {}, [](const Http::Request &req, Http::Response &res, auto next)
               {
        std::string action = req.getQuery("action");
        std::string name = req.getQuery("name");
        std::string value = req.getQuery("value");
        
        if (action == "set" && !name.empty()) {
            Http::CookieOptions opts;
            opts.maxAge = std::chrono::hours(1);
            res.setCookie(name, value, opts);
        } else if (action == "remove" && !name.empty()) {
            res.removeCookie(name);
        }
        
        res.TemporaryRedirect("/cookies"); });

    server.Get("/test/:id", {}, [](const Http::Request &req, Http::Response &res, auto next)
               {
        Http::CookieOptions secureOpts;
        res.setSignedCookie("secure", "ITS VERY SAFE", "123", secureOpts);
        res << 200 << "Test ID: " << req.getParam("id") << " Cookie: " << res.getSignedCookie("secure", "123").value_or(""); });

    server.Post("/upload", {}, [](const Http::Request &req, Http::Response &res, auto next)
                {
        auto fileData = req.getFormData("file");
        if (fileData.isFile && !fileData.file.empty()) {
            fileData.file.save("./public/" + fileData.filename);
            res << 200 << "File uploaded successfully: " << fileData.filename;
        } else {
            res << 400 << "File upload failed.";
        } });

    server.Post("/json", {}, [](const Http::Request &req, Http::Response &res, auto next)
                {
        const std::string jsonResponse = R"({"message": "JSON POST successful!"})";
        res << 200 << Json::ParseAndReturnBody(jsonResponse); });

    server.Get("/image-test", {}, [](const Http::Request &req, Http::Response &res, auto next)
               { res.SendFile("./public/a.jpg"); });

    Middleware authMiddleware = Middleware([](Http::Request &req, Http::Response &res, auto next)
                                           {
        std::string token = req.getQuery("token");
        if (token != "123") {
            res << 401 << "Unauthorized";
            return;
        }
        next(); });

    server["GET"].Use("/protected", {authMiddleware}, [](const Http::Request &req, Http::Response &res, auto next)
                      { res << 200 << Json::ParseAndReturnBody(R"({"message": "Protected area - Welcome!"})"); });

    server["GET"].Use("/redirect", {authMiddleware}, [](const Http::Request &req, Http::Response &res, auto next)
                      { res.MovedRedirect("/home"); });

    server["GET"].Register("/register-test").Use(authMiddleware).Then([](const Http::Request &req, Http::Response &res, auto next)
                                                                      { res << 200 << Json::ParseAndReturnBody(R"({"message": "Register test successful!"})"); });

    server.Get("/secure")
        .Use(authMiddleware)
        .Then([](const Http::Request &req, Http::Response &res, auto next)
              { res << 200 << Json::ParseAndReturnBody(R"({"message": "Secure area", "access": "granted"})"); });

    Router apiRouter;
    apiRouter.Get("/users", {}, [](const Http::Request &req, Http::Response &res, auto next)
                  { res << 200 << "User list"; });

    apiRouter.Get("/users/:id", {}, [](const Http::Request &req, Http::Response &res, auto next)
                  { res << 200 << "User ID: " << req.getParam("id"); });

    server.Use("/api", apiRouter);

    server.Group("/api/v1").Then([](Router &r)
                                 {
        r.Get("/users").Then([](const Http::Request &req, Http::Response &res, auto next) {
            res << 200 << "API v1 - Users";
        });
        r.Get("/posts").Then([](const Http::Request &req, Http::Response &res, auto next) {
            res << 200 << "API v1 - Posts";
        }); });

    server.Group("/admin").Then([](Router &r)
                                {
        r.Get("/dashboard").Then([](const Http::Request &req, Http::Response &res, auto next) {
            res << 200 << "Admin Dashboard";
        });
        r.Get("/settings").Then([](const Http::Request &req, Http::Response &res, auto next) {
            res << 200 << "Admin Settings";
        }); });

    server.Group("/blog").Then([](Router &r)
                               {
        r.Get("/posts").Then([](const Http::Request &req, Http::Response &res, auto next) {
            res << 200 << "Blog Posts";
        });
        r.Get("/posts/:id").Then([](const Http::Request &req, Http::Response &res, auto next) {
            res << 200 << "Blog post ID: " << req.getParam("id");
        });
        r.Get("/categories").Then([](const Http::Request &req, Http::Response &res, auto next) {
            res << 200 << "Blog Categories";
        }); });

    server.Get("/products").Then([](const Http::Request &req, Http::Response &res, auto next)
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
                {{"id", "101"}, {"name", "Smartphone"}, {"price", 7999.90}, {"inStock", true}},
                {{"id", "205"}, {"name", "Laptop"}, {"price", 12499.99}, {"inStock", false}},
                {{"id", "302"}, {"name", "Wireless Headphones"}, {"price", 1299.50}, {"inStock", true}}
            }},
            {"features", {"Fast Delivery", "Free Returns", "Original Product Guarantee"}}
        };
        res.Render("productPage", data); });

    server.Get("/middleware-demo", {}, [](const Http::Request &req, Http::Response &res, auto next)
               {
        std::cout << "First middleware: Logging request to " << req.path << std::endl;
        next(); });

    server.Get("/middleware-demo", {}, [](const Http::Request &req, Http::Response &res, auto next)
               {
        std::cout << "Second middleware: Adding custom header" << std::endl;
        res.setHeader("X-Custom-Header", "Nerva-Server");
        next(); });

    server.Get("/middleware-demo", {}, [](const Http::Request &req, Http::Response &res, auto next)
               {
        std::cout << "Final handler: Sending response" << std::endl;
        res << 200 << "Middleware demo completed! Check console for logs."; });

    server.Get("/auth-demo", {}, [](const Http::Request &req, Http::Response &res, auto next)
               {
        std::string token = req.getQuery("token");
        if (token == "secret123") {
            std::cout << "Authentication successful" << std::endl;
            next();
        } else {
            std::cout << "Authentication failed" << std::endl;
            res << 401 << "Unauthorized - Invalid token";
        } });

    server.Get("/auth-demo", {}, [](const Http::Request &req, Http::Response &res, auto next)
               { res << 200 << "Welcome to protected area! Token was valid."; });

    server.Get("/myip", {}, [](const Http::Request &req, Http::Response &res, auto next)
               { res << 200 << "Your ip is: " << req.ip << "\nYour ipv6 is: " << req.ipv6; });

    // Catch-all route for 404 - must be the last route
    server.Get("/*").Then([](const Http::Request &req, Http::Response &res, auto next)
                          { res.Render("notFound", nlohmann::json{}); });

    server.Start();
    server.Stop();
    return 0;
}