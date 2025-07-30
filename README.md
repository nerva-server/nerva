# Nerva HTTP Server

A high-performance, multi-threaded HTTP server written in C++20 with modern features including middleware support, static file serving, and JSON handling.

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
- **Enhanced Routing**: Advanced routing with method-specific handlers and chaining

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
    res << 200 << "Hello, World!";
});
```

### Route with Parameters

```cpp
server.Get("/test/:id", {}, [](const Http::Request &req, Http::Response &res) {
    res << 200 << "Test ID: " << req.getParam("id");
});
```

### JSON Response

```cpp
server.Get("/api/data", {}, [](const Http::Request &req, Http::Response &res) {
    const std::string jsonResponse = R"({"message": "Hello, API!"})";
    res << 200 << Json::ParseAndReturnBody(jsonResponse);
});
```

### Middleware Authentication

```cpp
Middleware login = Middleware([](Http::Request &req, Http::Response &res, auto next) {
    std::string token = req.getQuery("token");
    if (token != "123") {
        res << 401 << "Unauthorized";
        return;
    }
    next();
});

server.Get("/protected", {login}, [](const Http::Request &req, Http::Response &res) {
    res << 200 << "Protected content";
});
```

### Static File Serving

```cpp
server.Static("/static", "./public");
```

### Direct File Serving

```cpp
server.Get("/image").Then([](const Http::Request &req, Http::Response &res) {
    res.SendFile("./public/image.jpg");
});
```

### HTTP Redirects

```cpp
// Permanent redirect (301)
server.Get("/old-page", {}, [](const Http::Request &req, Http::Response &res) {
    res.MovedRedirect("/new-page");
});

// Temporary redirect (302)
server.Get("/temp-page", {}, [](const Http::Request &req, Http::Response &res) {
    res.TemporaryRedirect("/temporary-location");
});
```

### Advanced Routing with Method-Specific Handlers

```cpp
// Method-specific routing
server["GET"].Use("/api/data", {login}, [](const Http::Request &req, Http::Response &res) {
    res << 200 << "GET data";
});

// Chained routing with Register
server["GET"].Register("/registerTest").Use(login).Then([](const Http::Request &req, Http::Response &res) {
    res << 200 << "Registered route";
});
```

### Wildcard Routes and 404 Handling

```cpp
// Catch-all route for 404 handling
server.Get("/*").Then([](const Http::Request &req, Http::Response &res) {
    res << 404 << "Page not found";
});
```

### POST Request Handling

```cpp
server.Post("/api/submit", {}, [](const Http::Request &req, Http::Response &res) {
    // Handle POST data
    res << 200 << "Data received";
});
```

### Router Integration

```cpp
Router router;
router.Get("/router-test/:id", {}, [](const Http::Request &req, Http::Response &res) {
    res << 200 << "Router test: " << req.getParam("id");
});

server.Use("/api", router);
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
└── Makefile          # Build configuration
```

## API Reference

### Server Class

- `Server(ServerConfig &config)`: Initialize server with configuration
- `void Start()`: Start the server
- `void Stop()`: Stop the server
- `void Static(path, directory)`: Serve static files

### Router Methods

- `Get(path, middleware, handler)`: Register GET route
- `Post(path, middleware, handler)`: Register POST route
- `Use(path, middleware)`: Apply middleware to path
- `Register(path)`: Register a route for chaining
- `["METHOD"].Use(path, middleware, handler)`: Method-specific routing

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