# Nerva HTTP Server

A high-performance, multi-threaded HTTP server written in C++20 with modern features including middleware support, static file serving, JSON handling with simdjson and nlohmann/json, advanced template engine, clustering capabilities, and sophisticated routing capabilities.

## Features

- **High Performance**: Multi-threaded architecture with epoll-based event handling
- **Middleware Support**: Flexible middleware system for authentication and request processing
- **Static File Serving**: Built-in static file handler for serving public assets
- **JSON Support**: Integrated JSON parsing with simdjson for high-performance parsing and nlohmann/json for template engine data binding
- **Route Parameters**: Dynamic route parameter extraction (e.g., `/test/:id`)
- **Authentication**: Token-based authentication middleware
- **Thread Pool**: Configurable thread pool for handling concurrent connections
- **Keep-Alive**: HTTP keep-alive support for better performance
- **File Serving**: Direct file serving with automatic MIME type detection
- **HTTP Redirects**: Support for permanent (301) and temporary (302) redirects
- **Wildcard Routes**: Catch-all routes for 404 handling and fallback patterns
- **Enhanced Routing**: Advanced routing with method-specific handlers, chaining, and grouping
- **Advanced Template Engine**: Dynamic HTML rendering with nlohmann/json data binding
- **Template Include System**: Modular template structure with include support
- **Template Conditionals**: `{{ if }}` and `{{ endif }}` for conditional rendering
- **Template Loops**: `{{ for }}` and `{{ endfor }}` for iterative rendering
- **Template Filters**: Custom filters like `|formatPrice`, `|add:1` for data transformation
- **Template Caching**: Compiled template caching for faster rendering
- **Memory-Optimized Rendering**: Direct response writing without intermediate string copies
- **Custom 404 Pages**: Render custom not found pages with templates
- **Router Integration**: Modular API design with Router objects
- **Group Routing**: Route grouping for API versioning and modular structure
- **Memory Optimization**: tcmalloc integration for better memory management
- **Clustering Support**: Fork-based worker processes for load distribution
- **Configuration System**: Dynamic configuration with `.nrvcfg` files
- **Library Build**: Shared library support with install system
- **Dosya Yükleme**: Multipart/form-data desteği ile dosya upload (Request::getFormData, File::save)

## Quick Start

### Prerequisites

- C++20 compatible compiler (clang++ recommended)
- Linux system (uses epoll)
- simdjson library (for high-performance JSON parsing)
- nlohmann/json library (for template engine data binding)
- tcmalloc (optional, for better performance)

### Building

```bash
# Build executable
make

# Build shared library
make lib

# Install library and headers
make install
```

### Running

```bash
make run
```

The server will start on port 8080 by default with tcmalloc preloaded for optimal performance.

### Configuration

Create a `server.nrvcfg` file for custom configuration:

```cfg
server {
    port = 8080;
    buffer_size = 4096;
    thread_pool_size = 48;
    keep_alive_timeout = default;
    cluster_thread = 3;
    max_connections = 500000;
    accept_queue_size = 65535;
    accept_retry_delay_ms = 10;
    max_events = 8192;
}
```

## Usage Examples

### Basic Route

```cpp
#include <nerva/Server.hpp>
#include <nerva/Middleware.hpp>
#include <nerva/Json.hpp>

int main() {
    Server server = Server();
    server.SetConfigFile("server");

    server.Get("/", {}, [](const Http::Request &req, Http::Response &res) {
        res << 200 << "Home Page - Nerva HTTP Server";
    });

    server.Start();
    server.Stop();
    return 0;
}
```

### Route with Parameters

```cpp
server.Get("/test/:id", {}, [](const Http::Request &req, Http::Response &res) {
    res << 200 << "Test ID: " << req.getParam("id");
});
```

### JSON Response (POST) with simdjson

```cpp
server.Post("/test", {}, [](const Http::Request &req, Http::Response &res) {
    const std::string jsonResponse = R"({"message": "Test POST successful!"})";
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
    res << 200 << Json::ParseAndReturnBody(R"({"message": "Protected area - Welcome!"})");
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
    res << 200 << Json::ParseAndReturnBody(R"({"message": "Register test successful!"})");
});

server.Get("/secure")
    .Use(authMiddleware)
    .Then([](const Http::Request &req, Http::Response &res) {
        res << 200 << Json::ParseAndReturnBody(R"({"message": "Secure area", "access": "granted"})");
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

// Group with middleware support
server.Group("/testGroup", {}, [](Router &r) {
    r.Get("/users").Then([](const Http::Request &req, Http::Response &res) {
        res << 200 << "Protected Users";
    });
    r.Get("/posts").Then([](const Http::Request &req, Http::Response &res) {
        res << 200 << "Protected Posts";
    });
});
```

### Advanced Template Engine with nlohmann/json

```cpp
Nerva::Engine *engine = new Nerva::Engine();
engine->setViewsDirectory("./views");
server.Set("view engine", engine);

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
    
    res.Render("productPage", data);
});
```

### Template Features

#### Template Include System
```html
{{ include header }}
{{ include productCard with product }}
{{ include footer }}
```

#### Template Conditionals
```html
{{ if showPromo }}
<div class="promo-banner">
    {{ promoMessage }}
</div>
{{ endif }}

{{ if user.premium }}
    <span class="badge premium">PREMIUM</span>
{{ endif }}
```

#### Template Loops
```html
{{ for product in products }}
    {{ include productCard with product }}
{{ endfor }}

{{ for feature, index in features }}
    <li>{{ index|add:1 }}. {{ feature }}</li>
{{ endfor }}
```

#### Template Filters
```html
<p class="price">${{ product.price|formatPrice }}</p>
<li>{{ index|add:1 }}. {{ feature }}</li>
```

### Custom 404 Page (Wildcard Route)

```cpp
server.Get("/*").Then([](const Http::Request &req, Http::Response &res) {
    res.Render("notFound", nlohmann::json{});
});
```

### Dosya Yükleme (Multipart Form Data)

```cpp
server.Post("/upload", {}, [](const Http::Request &req, Http::Response &res) {
    auto fileData = req.getFormData("file");
    if (fileData.isFile && !fileData.file.empty()) {
        fileData.file.save("./public/" + fileData.filename);
        res << 200 << "Dosya başarıyla yüklendi: " << fileData.filename;
    } else {
        res << 400 << "Dosya yüklenemedi.";
    }
});
```

#### HTML Form Örneği
```html
<form action="/upload" method="post" enctype="multipart/form-data">
  <input type="file" name="file">
  <button type="submit">Yükle</button>
</form>
```

### Gelişmiş Route ve Middleware Kullanımı

```cpp
// Middleware ile korunan route
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
```

### Tüm Özellikleri Kapsayan main.cpp Örneği

```cpp
#include <iostream>
#include "Server.hpp"
#include "Middleware.hpp"
#include "Json.hpp"
#include "ViewEngine/NervaEngine.hpp"

int main() {
    Server server;
    server.SetConfigFile("server");
    server.Static("/static", "./public");
    Nerva::Engine *engine = new Nerva::Engine();
    engine->setViewsDirectory("./views");
    server.Set("view engine", engine);

    server.Get("/", {}, [](const Http::Request &req, Http::Response &res) {
        res << 200 << "Home Page - Nerva HTTP Server";
    });
    server.Get("/test/:id", {}, [](const Http::Request &req, Http::Response &res) {
        res << 200 << "Test ID: " << req.getParam("id");
    });
    server.Post("/upload", {}, [](const Http::Request &req, Http::Response &res) {
        auto fileData = req.getFormData("file");
        if (fileData.isFile && !fileData.file.empty()) {
            fileData.file.save("./public/" + fileData.filename);
            res << 200 << "Dosya başarıyla yüklendi: " << fileData.filename;
        } else {
            res << 400 << "Dosya yüklenemedi.";
        }
    });
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
    server.Get("/products").Then([](const Http::Request &req, Http::Response &res) {
        nlohmann::json data = {
            {"pageTitle", "Super Products"},
            {"products", {{{"id", "101"}, {"name", "Smartphone"}, {"price", 7999.90}, {"inStock", true}}}}
        };
        res.Render("productPage", data);
    });
    server.Get("/*").Then([](const Http::Request &req, Http::Response &res) {
        res.Render("notFound", nlohmann::json{});
    });
    server.Start();
    server.Stop();
    return 0;
}
```

## Configuration

The server uses a custom configuration system with `.nrvcfg` files:

### Configuration Parameters

- **port**: Server port (default: 8080)
- **buffer_size**: Request buffer size (default: 4096)
- **thread_pool_size**: Number of worker threads (default: 48)
- **keep_alive_timeout**: Keep-alive timeout in seconds (default: 5)
- **cluster_thread**: Number of cluster worker processes (default: 3)
- **max_connections**: Maximum concurrent connections (default: 500000)
- **accept_queue_size**: TCP accept queue size (default: 65535)
- **accept_retry_delay_ms**: Accept retry delay in milliseconds (default: 10)
- **max_events**: Maximum epoll events (default: 8192)

### Configuration File Format

```cfg
server {
    port = 8080;
    buffer_size = 4096;
    thread_pool_size = 48;
    keep_alive_timeout = 5;
    cluster_thread = 3;
    max_connections = 500000;
    accept_queue_size = 65535;
    accept_retry_delay_ms = 10;
    max_events = 8192;
}
```

### Dynamic Configuration Loading

```cpp
Server server = Server();
server.SetConfigFile("server"); // Loads server.nrvcfg
```

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
│   │   └── Config/    # Configuration parser
│   ├── Utils/         # Utility functions
│   ├── Radix/         # Radix tree implementation
│   └── ViewEngine/    # Template engine system
├── src/               # Source files
├── public/            # Static files
├── views/             # HTML templates for view engine
│   ├── header.html    # Header template
│   ├── footer.html    # Footer template
│   ├── productCard.html # Product card component
│   ├── productPage.html # Product page template
│   └── notFound.html  # 404 page template
├── server.nrvcfg      # Server configuration file
├── lib/               # Library build directory
└── Makefile           # Build configuration
```

## API Reference

### Server Class

- `Server()`: Initialize server with default configuration
- `Server(ServerConfig &config)`: Initialize server with custom configuration
- `void Start()`: Start the server
- `void Stop()`: Stop the server
- `void Static(path, directory)`: Serve static files
- `void Set(key, value)`: Set server options (e.g., view engine, views directory)
- `void Use(path, Router)`: Mount a Router at a path
- `Group(path)`: Start a route group for modular routing
- `Group(path, middlewares, handler)`: Start a route group with middleware
- `void SetConfigFile(path)`: Set configuration file path

### Router Methods

- `Get(path, middleware, handler)`: Register GET route
- `Post(path, middleware, handler)`: Register POST route
- `Use(path, middleware)`: Apply middleware to path
- `Register(path)`: Register a route for chaining
- `["METHOD"].Use(path, middleware, handler)`: Method-specific routing
- `Then(handler)`: Chain handler after middleware or group
- `Group(path)`: Create a route group
- `Group(path, middlewares, handler)`: Create a route group with middleware

### GroupBuilder Class

- `Use(middleware)`: Add middleware to the group
- `Then(handler)`: Execute the group handler with middleware

### Request Object

- `getParam(name)`: Get route parameter
- `getQuery(name)`: Get query parameter
- `getHeader(name)`: Get request header
- `getBody()`: Get request body
- `const FormData &Request::getFormData(const std::string &key) const`: Multipart form alanı veya dosya verisini döndürür.
- `bool File::save(const std::string &path) const`: Yüklenen dosyayı belirtilen yola kaydeder.

### Response Object

- `<< status << content`: Send response with status code and content
- `SendFile(path)`: Serve a file directly with MIME type detection
- `MovedRedirect(location)`: Send 301 permanent redirect
- `TemporaryRedirect(location)`: Send 302 temporary redirect
- `setHeader(key, value)`: Set custom response header
- `Render(view, data)`: Render a template with nlohmann::json data

### Template Engine Features

- **Include System**: `{{ include templateName }}`
- **Include with Context**: `{{ include templateName with variable }}`
- **Conditionals**: `{{ if condition }}...{{ endif }}`
- **Loops**: `{{ for item in collection }}...{{ endfor }}`
- **Filters**: `{{ value|filter:param }}`
- **Data Binding**: Direct nlohmann::json object access
- **Template Caching**: Automatic template caching for performance

### JSON Utilities

- `Json::ParseAndReturnBody(jsonString)`: Parse and return JSON string using simdjson
- `nlohmann::json`: Modern JSON library for template engine data binding

### Clustering System

- `Cluster`: Fork-based worker process management
- `forkWorkers(serverSocket, cpuCount)`: Create worker processes
- `sendShutdownSignal(workers)`: Graceful shutdown of workers
- `waitForWorkers(workers)`: Wait for worker termination

## Performance

The server is optimized for high-performance scenarios:

- **Epoll-based I/O**: Efficient event-driven I/O handling
- **Thread Pool**: Configurable worker thread pool
- **Non-blocking Sockets**: Asynchronous connection handling
- **Memory Efficient**: Uses modern C++ features for optimal memory usage
- **MIME Type Detection**: Automatic content-type detection for files
- **Radix Tree Routing**: Fast route matching with parameter extraction
- **tcmalloc Integration**: High-performance memory allocator
- **Template Caching**: Compiled template caching for faster rendering
- **High-Performance JSON**: simdjson for fast JSON parsing
- **Clustering**: Fork-based worker processes for load distribution
- **Dynamic Configuration**: Runtime configuration loading
- **Memory-Optimized Template Rendering**: Direct response writing without intermediate string copies

### Benchmark Results

The server has been tested with `wrk` benchmark tool under high load conditions:

```bash
wrk -t6 -c2000 -d10s http://localhost:8080/test
```

**Test Configuration:**
- **Threads**: 6 worker threads
- **Connections**: 2000 concurrent connections
- **Duration**: 10 seconds
- **Target**: 404 Not Found endpoint

**Results:**
```
Running 10s test @ http://localhost:8080/test
  6 threads and 2000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     2.66ms   12.56ms 248.06ms   95.03%
    Req/Sec    34.53k    21.90k   82.11k    60.00%
  2061090 requests in 10.09s, 2.98GB read
Requests/sec: 204,350.42
Transfer/sec:    302.65MB
```

**Performance Metrics:**
- **Throughput**: 204,350 requests/second
- **Average Latency**: 2.66ms
- **Total Requests**: 2,061,090 requests in 10.09 seconds
- **Data Transfer**: 302.65MB/second
- **Connection Handling**: 2000 concurrent connections
- **CPU Utilization**: 6 threads efficiently utilized

**Key Performance Features:**
- **High Throughput**: Over 200K requests/second
- **Low Latency**: Sub-3ms average response time
- **Concurrent Handling**: 2000 simultaneous connections
- **Memory Efficiency**: Optimized memory usage with tcmalloc
- **Scalable Architecture**: Fork-based clustering for load distribution

## Template Engine Architecture

The Nerva template engine provides:

- **Modular Design**: Reusable template components (header, footer, cards)
- **Data Binding**: Seamless integration with nlohmann::json
- **Conditional Rendering**: Dynamic content based on data
- **Iterative Rendering**: Loop through collections and arrays
- **Custom Filters**: Extensible filter system for data transformation
- **Include System**: Template composition and reuse with context passing
- **CSS Integration**: Inline styling support in templates
- **Template Caching**: Automatic caching for improved performance
- **Memory-Optimized Rendering**: Direct response writing without intermediate string copies

### Memory Optimization

The template engine has been optimized for memory efficiency:

**Before (Memory Inefficient):**
```cpp
std::string render(const std::string &templateName, const json &context);
// Returns string, requires additional memory copy
```

**After (Memory Optimized):**
```cpp
void render(Http::Response &res, const std::string &templateName, const json &context);
// Directly writes to response body, no intermediate copies
```

**Benefits:**
- **Reduced Memory Usage**: Eliminates intermediate string copies
- **Faster Rendering**: Direct response writing
- **Lower GC Pressure**: Less temporary object creation
- **Better Performance**: Especially under high load

## Clustering Architecture

The Nerva clustering system provides:

- **Fork-based Workers**: Multiple worker processes for load distribution
- **CPU-based Scaling**: Automatic worker count based on CPU cores
- **Graceful Shutdown**: Proper signal handling and cleanup
- **Process Management**: Automatic worker process lifecycle management
- **Load Distribution**: Shared socket across worker processes

## JSON Libraries

The server uses two JSON libraries for different purposes:

- **simdjson**: High-performance JSON parsing for `Json::ParseAndReturnBody()` function
- **nlohmann/json**: Modern JSON library for template engine data binding and rendering

## Build System

The project supports multiple build targets:

- **Executable**: `make` - Builds the main server executable
- **Shared Library**: `make lib` - Builds `nerva.so` shared library
- **Install**: `make install` - Installs library and headers to system
- **Clean**: `make clean` - Removes build artifacts

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
- simdjson library (for high-performance JSON parsing)
- nlohmann/json library (for template engine data binding)
- tcmalloc (optional, for better performance)

---

**Made with ❤️ and ☕** 