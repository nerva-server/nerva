# Nerva HTTP Server

A high-performance, multi-threaded HTTP server written in C++20 with modern features including middleware support, static file serving, JSON handling, view engine (template) support, and advanced routing.

## Features

- **High Performance**: Multi-threaded architecture with epoll-based event handling
- **Middleware Support**: Flexible middleware system for authentication and request processing
- **Static File Serving**: Built-in static file handler for serving public assets
- **JSON Support**: Integrated JSON parsing and response handling
- **Route Parameters**: Dynamic route parameter extraction (e.g., `/test/:id`)
- **Authentication**: Token-based authentication middleware
- **Thread Pool**: Configurable thread pool for handling concurrent connections
- **Keep-Alive**: HTTP keep-alive support for better performance
- **File Serving**: Direct file serving with automatic MIME type detection
- **HTTP Redirects**: Support for permanent (301) and temporary (302) redirects
- **Wildcard Routes**: Catch-all routes for 404 handling and fallback patterns
- **Enhanced Routing**: Advanced routing with method-specific handlers, chaining, and grouping
- **View Engine Support**: Dynamic HTML rendering with template engine and data binding
- **Custom 404 Pages**: Render custom not found pages with templates
- **Router Integration**: Modular API design with Router objects
- **Group Routing**: Route grouping for API versioning and modular structure

## Quick Start

### Prerequisites

- C++20 compatible compiler (clang++ recommended)
- Linux system (uses epoll)
- libsimdjson for JSON parsing

### Building

```bash
make
```

### Running

```bash
make run
```

The server will start on port 8080 by default.

## Usage Examples

### Basic Route

```cpp
server.Get("/", {}, [](const Http::Request &req, Http::Response &res) {
    res << 200 << "Home Page - Nerva HTTP Server";
});
```

### Route with Parameters

```cpp
server.Get("/test/:id", {}, [](const Http::Request &req, Http::Response &res) {
    res << 200 << "Test ID: " << req.getParam("id");
});
```

### JSON Response (POST)

```cpp
server.Post("/test", {}, [](const Http::Request &req, Http::Response &res) {
    const std::string jsonResponse = R"({\"message\": \"Test POST successful!\"})";
    res << 200 << Json::ParseAndReturnBody(jsonResponse);
});
```

### Middleware Authentication

```cpp
Middleware authMiddleware = Middleware([](Http::Request &req, Http::Response &res, auto next) {
    std::string token = req.getQuery("token");
    if (token != "123") {
        res << 401 << "Unauthorized";
        return;
    }
    next();
});

server["GET"].Use("/protected", {authMiddleware}, [](const Http::Request &req, Http::Response &res) {
    res << 200 << Json::ParseAndReturnBody(R"({\"message\": \"Protected area - Welcome!\"})");
});
```

### Static File Serving

```cpp
server.Static("/static", "./public");
```

### Direct File Serving

```cpp
server.Get("/image-test", {}, [](const Http::Request &req, Http::Response &res) {
    res.SendFile("./public/a.jpg");
});
```

### HTTP Redirects (with Middleware)

```cpp
server["GET"].Use("/redirect", {authMiddleware}, [](const Http::Request &req, Http::Response &res) {
    res.MovedRedirect("/home");
});
```

### Advanced Routing: Register, Use, Then

```cpp
server["GET"].Register("/register-test").Use(authMiddleware).Then([](const Http::Request &req, Http::Response &res) {
    res << 200 << Json::ParseAndReturnBody(R"({\"message\": \"Register test successful!\"})");
});

server.Get("/secure")
    .Use(authMiddleware)
    .Then([](const Http::Request &req, Http::Response &res) {
        res << 200 << Json::ParseAndReturnBody(R"({\"message\": \"Secure area\", \"access\": \"granted\"})");
    });
```

### Router Integration

```cpp
Router apiRouter;
apiRouter.Get("/users", {}, [](const Http::Request &req, Http::Response &res) {
    res << 200 << "User list";
});
apiRouter.Get("/users/:id", {}, [](const Http::Request &req, Http::Response &res) {
    res << 200 << "User ID: " << req.getParam("id");
});
server.Use("/api", apiRouter);
```

### Group Routing (API Versioning, Admin, Blog, etc.)

```cpp
server.Group("/api/v1").Then([](Router &r) {
    r.Get("/users").Then([](const Http::Request &req, Http::Response &res) {
        res << 200 << "API v1 - Users";
    });
    r.Get("/posts").Then([](const Http::Request &req, Http::Response &res) {
        res << 200 << "API v1 - Posts";
    });
});

server.Group("/admin").Then([](Router &r) {
    r.Get("/dashboard").Then([](const Http::Request &req, Http::Response &res) {
        res << 200 << "Admin Dashboard";
    });
    r.Get("/settings").Then([](const Http::Request &req, Http::Response &res) {
        res << 200 << "Admin Settings";
    });
});
```

### View Engine: Dynamic HTML Rendering

```cpp
Nerva::Engine *engine = new Nerva::Engine();
engine->setViewsDirectory("./views");
server.Set("views", "./views");
server.Set("view engine", engine);

server.Get("/products").Then([](const Http::Request &req, Http::Response &res) {
    auto data = std::map<std::string, std::shared_ptr<Nerva::Value>>{
        {"pageTitle", Nerva::createValue("Super Products")},
        {"showPromo", Nerva::createValue(true)},
        {"promoMessage", Nerva::createValue("TODAY'S SPECIAL DISCOUNT!")},
        {"user", std::make_shared<Nerva::ObjectValue>(std::map<std::string, std::shared_ptr<Nerva::Value>>{
            {"name", Nerva::createValue("Ayşe Demir")},
            {"premium", Nerva::createValue(true)},
            {"cartItems", Nerva::createValue("3")}})},
        {"products", Nerva::createArray(std::vector<std::map<std::string, std::shared_ptr<Nerva::Value>>>{
            {{"id", Nerva::createValue("101")}, {"name", Nerva::createValue("Smartphone")}, {"price", Nerva::createValue(7999.90)}, {"inStock", Nerva::createValue(true)}},
            {{"id", Nerva::createValue("205")}, {"name", Nerva::createValue("Laptop")}, {"price", Nerva::createValue(12499.99)}, {"inStock", Nerva::createValue(false)}},
            {{"id", Nerva::createValue("302")}, {"name", Nerva::createValue("Wireless Headphones")}, {"price", Nerva::createValue(1299.50)}, {"inStock", Nerva::createValue(true)}}
        })},
        {"features", Nerva::createArray(std::vector<std::string>{
            "Fast Delivery", "Free Returns", "Original Product Guarantee"})}
    };
    res.Render("productPage", data);
});
```

### Custom 404 Page (Wildcard Route)

```cpp
server.Get("/*").Then([](const Http::Request &req, Http::Response &res) {
    res.Render("notFound", std::map<std::string, std::shared_ptr<Nerva::Value>>{});
});
```

## Configuration

The server configuration is defined in `ServerConfig.hpp`:

- **PORT**: Server port (default: 8080)
- **BUFFER_SIZE**: Request buffer size (default: 4096)
- **THREAD_POOL_SIZE**: Number of worker threads (default: 48)
- **KEEP_ALIVE_TIMEOUT**: Keep-alive timeout in seconds (default: 5)
- **MAX_CONNECTIONS**: Maximum concurrent connections (default: 500000)
- **ACCEPT_QUEUE_SIZE**: TCP accept queue size (default: 65535)

## Project Structure

```
Nerva/
├── includes/           # Header files
│   ├── Core/          # Core server components
│   │   ├── Server/    # Server implementation
│   │   ├── Http/      # HTTP handling
│   │   │   ├── Request/   # Request/Response classes
│   │   │   ├── Router/    # Routing system
│   │   │   ├── Handler/   # Request handlers
│   │   │   └── Middleware/ # Middleware system
│   │   └── Cluster/   # Clustering support
│   ├── Secure/        # Security and configuration
│   ├── Utils/         # Utility functions
│   └── Radix/         # Radix tree implementation
├── src/               # Source files
├── public/            # Static files
├── views/             # HTML templates for view engine
└── Makefile           # Build configuration
```

## API Reference

### Server Class

- `Server(ServerConfig &config)`: Initialize server with configuration
- `void Start()`: Start the server
- `void Stop()`: Stop the server
- `void Static(path, directory)`: Serve static files
- `void Set(key, value)`: Set server options (e.g., view engine, views directory)
- `void Use(path, Router)`: Mount a Router at a path
- `Group(path)`: Start a route group for modular routing

### Router Methods

- `Get(path, middleware, handler)`: Register GET route
- `Post(path, middleware, handler)`: Register POST route
- `Use(path, middleware)`: Apply middleware to path
- `Register(path)`: Register a route for chaining
- `["METHOD"].Use(path, middleware, handler)`: Method-specific routing
- `Then(handler)`: Chain handler after middleware or group

### Request Object

- `getParam(name)`: Get route parameter
- `getQuery(name)`: Get query parameter
- `getHeader(name)`: Get request header
- `getBody()`: Get request body

### Response Object

- `<< status << content`: Send response with status code and content
- `SendFile(path)`: Serve a file directly with MIME type detection
- `MovedRedirect(location)`: Send 301 permanent redirect
- `TemporaryRedirect(location)`: Send 302 temporary redirect
- `setHeader(key, value)`: Set custom response header
- `Render(view, data)`: Render a template with data (view engine)

### JSON Utilities

- `Json::ParseAndReturnBody(jsonString)`: Parse and return JSON string

## Performance

The server is optimized for high-performance scenarios:

- **Epoll-based I/O**: Efficient event-driven I/O handling
- **Thread Pool**: Configurable worker thread pool
- **Non-blocking Sockets**: Asynchronous connection handling
- **Memory Efficient**: Uses modern C++ features for optimal memory usage
- **MIME Type Detection**: Automatic content-type detection for files
- **Radix Tree Routing**: Fast route matching with parameter extraction

## License

See LICENSE file for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## Requirements

- Linux (uses epoll)
- C++20 compiler
- libsimdjson
- tcmalloc (optional, for better performance)

---

**Made with ❤️ and ☕** 