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

### POST Request Handling

```cpp
server.Post("/api/submit", {}, [](const Http::Request &req, Http::Response &res) {
    // Handle POST data
    res << 200 << "Data received";
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

### Request Object

- `getParam(name)`: Get route parameter
- `getQuery(name)`: Get query parameter
- `getBody()`: Get request body

### Response Object

- `<< status << content`: Send response with status code and content

## Performance

The server is optimized for high-performance scenarios:

- **Epoll-based I/O**: Efficient event-driven I/O handling
- **Thread Pool**: Configurable worker thread pool
- **Non-blocking Sockets**: Asynchronous connection handling
- **Memory Efficient**: Uses modern C++ features for optimal memory usage

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