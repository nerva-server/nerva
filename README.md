# Nerva HTTP Server

A high-performance, multi-threaded HTTP server written in C++20 with modern features including middleware support, static file serving, JSON handling with simdjson and nlohmann/json, advanced template engine, clustering capabilities, and sophisticated routing capabilities.

## Table of Contents

- [Features](#features)
- [Quick Start](#quick-start)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage Examples](#usage-examples)
- [Template Engine](#template-engine)
- [API Reference](#api-reference)
- [Performance](#performance)
- [Architecture](#architecture)
- [Development](#development)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## Quick Links

- [🚀 Quick Start Guide](#quick-start)
- [📦 Installation](#installation)
- [⚙️ Configuration](#configuration)
- [💡 Usage Examples](#usage-examples)
- [🎨 Template Engine](#template-engine)
- [🔧 API Reference](#api-reference)
- [⚡ Performance](#performance)
- [🏗️ Architecture](#architecture)
- [🛠️ Development](#development)
- [🔍 Troubleshooting](#troubleshooting)

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
- **File Upload**: Multipart/form-data support for file upload (Request::getFormData, File::save)
- **HTTP Methods**: Full support for GET, POST, PUT, DELETE methods with chaining
- **Content-Type Detection**: Automatic content-type detection for responses
- **Thread-Safe Architecture**: Thread-safe queue and thread pool for concurrent connections
- **MIME Type Support**: Comprehensive MIME type detection for static files
- **Response Headers**: Custom header management and automatic header setting
- **Process Management**: Fork-based clustering with graceful shutdown
- **Non-blocking I/O**: Epoll-based event-driven architecture
- **Cookie Management**: Comprehensive cookie handling with signed cookies and session management

## Quick Start

### Prerequisites

- C++20 compatible compiler (clang++ recommended)
- Linux system (uses epoll)
- simdjson library (for high-performance JSON parsing)
- nlohmann/json library (for template engine data binding)
- tcmalloc (optional, for better performance)

### Getting Started in 5 Minutes

1. **Install Dependencies**
   ```bash
   sudo apt update
   sudo apt install clang++ libsimdjson-dev nlohmann-json3-dev libssl-dev libtcmalloc-minimal4 make
   ```

2. **Clone and Build**
   ```bash
   git clone https://github.com/yourusername/nerva.git
   cd nerva
   make
   ```

3. **Run the Server**
   ```bash
   make run
   ```

4. **Test the Server**
   ```bash
   # In another terminal
   curl http://localhost:8080/
   # Should return: "Home Page - Nerva HTTP Server"
   ```

5. **Create Your First Route**
   ```cpp
   // Edit src/main.cpp and add:
   server.Get("/hello", {}, [](const Http::Request &req, Http::Response &res) {
       res << 200 << "Hello, World!";
   });
   ```

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

## Installation

### System Requirements

- **Operating System**: Linux (uses epoll for event handling)
- **Compiler**: C++20 compatible (clang++ 12+ or g++ 10+)
- **Memory**: Minimum 512MB RAM, recommended 2GB+
- **Storage**: 100MB free space for build artifacts

### Dependencies Installation

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install clang++ libsimdjson-dev nlohmann-json3-dev libssl-dev libtcmalloc-minimal4
```

#### Arch Linux
```bash
sudo pacman -S clang simdjson nlohmann-json openssl gperftools
```

#### CentOS/RHEL/Fedora
```bash
sudo dnf install clang simdjson-devel nlohmann-json-devel openssl-devel gperftools-devel
```

### Building from Source

```bash
# Clone the repository
git clone https://github.com/yourusername/nerva.git
cd nerva

# Build the server
make

# Build shared library (optional)
make lib

# Install system-wide (optional)
sudo make install
```

### Docker Installation

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    clang++ \
    libsimdjson-dev \
    nlohmann-json3-dev \
    libssl-dev \
    libtcmalloc-minimal4 \
    make \
    git

WORKDIR /app
COPY . .

RUN make

EXPOSE 8080
CMD ["./server"]
```

### Verification

After installation, verify the server works:

```bash
# Start the server
make run

# In another terminal, test the server
curl http://localhost:8080/
# Should return: "Home Page - Nerva HTTP Server"

# Test JSON endpoint
curl -X POST http://localhost:8080/json
# Should return: {"message": "JSON POST successful!"}
```

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

### HTTP Methods Support

```cpp
// GET method
server.Get("/users", {}, [](const Http::Request &req, Http::Response &res) {
    res << 200 << "User list";
});

// POST method
server.Post("/users", {}, [](const Http::Request &req, Http::Response &res) {
    res << 201 << "User created";
});

// PUT method
server.Put("/users/:id", {}, [](const Http::Request &req, Http::Response &res) {
    res << 200 << "User updated: " << req.getParam("id");
});

// DELETE method
server.Delete("/users/:id", {}, [](const Http::Request &req, Http::Response &res) {
    res << 204 << "User deleted: " << req.getParam("id");
});
```

### Content-Type Detection and Response Headers

```cpp
server.Get("/api/data", {}, [](const Http::Request &req, Http::Response &res) {
    // Automatic content-type detection
    res << 200 << R"({"message": "JSON response"})";
    // Will automatically set Content-Type: application/json
});

server.Get("/custom", {}, [](const Http::Request &req, Http::Response &res) {
    res.setHeader("X-Custom-Header", "Custom Value");
    res.setHeader("Cache-Control", "no-cache");
    res << 200 << "Custom response with headers";
});
```

### Thread-Safe Architecture

The server uses a sophisticated thread-safe architecture:

- **Thread-Safe Queue**: Lock-free socket queue for connection handling
- **Thread Pool**: Configurable worker thread pool for request processing
- **Non-blocking I/O**: Epoll-based event-driven architecture
- **Connection Limits**: Configurable maximum connections and queue sizes

```cpp
// Configuration for thread pool and connection limits
server {
    thread_pool_size = 48;        // Worker threads
    max_connections = 500000;      // Max concurrent connections
    accept_queue_size = 65535;     // TCP accept queue
    max_events = 8192;            // Epoll events limit
}
```

### Process Management and Clustering

```cpp
// Fork-based worker processes
Cluster clusterManager;
std::vector<pid_t> workers = clusterManager.forkWorkers(serverSocket, cpuCount);

// Graceful shutdown
clusterManager.sendShutdownSignal(workers);
clusterManager.waitForWorkers(workers);
```

### MIME Type Support

The server automatically detects and serves files with proper MIME types:

```cpp
// Supported file types with automatic MIME detection
server.Static("/static", "./public");
// Automatically serves:
// - HTML files (.html, .htm) as text/html
// - CSS files (.css) as text/css
// - JavaScript files (.js) as text/javascript
// - Images (.png, .jpg, .gif, .svg) as image/*
// - JSON files (.json) as application/json
// - And many more...
```

### Advanced Response Features

```cpp
server.Get("/download", {}, [](const Http::Request &req, Http::Response &res) {
    res.setHeader("Content-Disposition", "attachment; filename=file.pdf");
    res.setHeader("Cache-Control", "no-cache");
    res.SendFile("./files/document.pdf");
});

server.Get("/redirect-permanent", {}, [](const Http::Request &req, Http::Response &res) {
    res.MovedRedirect("/new-location");  // 301 redirect
});

server.Get("/redirect-temporary", {}, [](const Http::Request &req, Http::Response &res) {
    res.TemporaryRedirect("/temp-location");  // 302 redirect
});
```

### Enhanced Request Handling and Performance

The server includes advanced request handling with several performance optimizations:

```cpp
// Enhanced request handling with timeout management
void Server::handleClient(int clientSocket) {
    // Configurable buffer size with memory pre-allocation
    const size_t BUFFER_SIZE = config.getInt("buffer_size");
    std::vector<char> buffer(BUFFER_SIZE);
    std::string requestData;
    requestData.reserve(BUFFER_SIZE * 2); // Pre-allocate memory
    
    // Socket timeout configuration
    struct timeval tv;
    tv.tv_sec = 5;  // 5 second timeout
    tv.tv_usec = 0;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    // Keep-alive support
    bool keepAlive = req.headers["Connection"] == "keep-alive" ||
                     (req.version == "HTTP/1.1" && req.headers["Connection"] != "close");
}
```

### Advanced Error Handling

```cpp
// Comprehensive error handling with proper HTTP responses
try {
    // Request parsing with validation
    if (!req.parse(requestData)) {
        std::string badReq = "HTTP/1.1 400 Bad Request\r\n"
                             "Connection: close\r\n"
                             "Content-Length: 0\r\n\r\n";
        send(clientSocket, badReq.data(), badReq.size(), MSG_NOSIGNAL);
        break;
    }
} catch (const std::exception &e) {
    std::cerr << "Client error: " << e.what() << std::endl;
}
```

### Connection Management and Performance

```cpp
// Socket optimization for high performance
int Server::initSocket(int port, int listenQueueSize) {
    // Non-blocking socket with optimized settings
    int sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    
    // TCP optimizations
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
    
    // Buffer size optimization (1MB)
    int bufsize = 1024 * 1024;
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
}
```

### Cookie Management and Session Handling

```cpp
// Set various types of cookies
server.Get("/cookies", {}, [](const Http::Request &req, Http::Response &res) {
    // Basic cookie
    Http::CookieOptions basicOpts;
    basicOpts.maxAge = std::chrono::hours(1);
    res.setCookie("basic_cookie", "Hello World", basicOpts);
    
    // Secure signed cookie
    Http::CookieOptions secureOpts;
    secureOpts.maxAge = std::chrono::hours(2);
    secureOpts.httpOnly = true;
    secureOpts.secure = false; // Set to true in production
    res.setSignedCookie("secure_cookie", "Secret Data", "my-secret-key", secureOpts);
    
    // Session cookie
    Http::CookieOptions sessionOpts;
    sessionOpts.maxAge = std::chrono::hours(24);
    sessionOpts.httpOnly = true;
    sessionOpts.sameSite = "Strict";
    res.setCookie("session_cookie", "User Session Data", sessionOpts);
    
    // Read cookies
    std::string basicValue = res.getCookieValue("basic_cookie", "Not Set");
    auto secureValue = res.getSignedCookie("secure_cookie", "my-secret-key");
    
    nlohmann::json data = {
        {"pageTitle", "Cookie Examples"},
        {"basicCookie", basicValue},
        {"secureCookie", secureValue.value_or("Invalid or Not Set")}
    };
    
    res.Render("cookies", data);
});

// Remove cookie
server.Get("/logout", {}, [](const Http::Request &req, Http::Response &res) {
    res.removeCookie("session_id");
    res.TemporaryRedirect("/login");
});
```

### Authentication System

```cpp
// Simple user database (in real app, use proper database)
std::map<std::string, std::string> users = {
    {"admin", "password123"},
    {"user1", "password456"},
    {"demo", "demo123"}
};

// Session storage (in real app, use Redis or database)
std::map<std::string, std::string> sessions;

// Login page
server.Get("/login", {}, [](const Http::Request &req, Http::Response &res) {
    nlohmann::json data = {
        {"pageTitle", "Login - Nerva HTTP Server"},
        {"error", ""}
    };
    res.Render("login", data);
});

// Login form handler
server.Post("/login", {}, [](const Http::Request &req, Http::Response &res) {
    std::string username = req.getFormData("username").value;
    std::string password = req.getFormData("password").value;
    
    // Check credentials
    if (users.find(username) != users.end() && users[username] == password) {
        // Generate session ID
        std::string sessionId = "sess_" + std::to_string(std::time(nullptr)) + "_" + username;
        sessions[sessionId] = username;
        
        // Set secure cookie
        Http::CookieOptions cookieOpts;
        cookieOpts.maxAge = std::chrono::hours(24); // 24 hours
        cookieOpts.httpOnly = true;
        cookieOpts.secure = false; // Set to true in production with HTTPS
        cookieOpts.sameSite = "Lax";
        
        res.setCookie("session_id", sessionId, cookieOpts);
        res.TemporaryRedirect("/dashboard");
    } else {
        // Invalid credentials
        nlohmann::json data = {
            {"pageTitle", "Login - Nerva HTTP Server"},
            {"error", "Invalid username or password"}
        };
        res.Render("login", data);
    }
});

// Protected dashboard
server.Get("/dashboard", {}, [](const Http::Request &req, Http::Response &res) {
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
    res.Render("dashboard", data);
});
```

### Cookie Management Tools

```cpp
// Cookie management example
server.Get("/cookie-manager", {}, [](const Http::Request &req, Http::Response &res) {
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
    
    res.TemporaryRedirect("/cookies");
});
```

### Example main.cpp Demonstrating All Features

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
            res << 200 << "File uploaded successfully: " << fileData.filename;
        } else {
            res << 400 << "File upload failed.";
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

### Configuration Optimization

#### High-Performance Configuration
```cfg
server {
    port = 8080;
    buffer_size = 8192;           # Larger buffer for high throughput
    thread_pool_size = 64;        # More threads for high concurrency
    keep_alive_timeout = 10;      # Longer keep-alive for better performance
    cluster_thread = 4;           # More worker processes
    max_connections = 1000000;    # Higher connection limit
    accept_queue_size = 131072;   # Larger accept queue
    accept_retry_delay_ms = 5;    # Faster retry
    max_events = 16384;           # More epoll events
}
```

#### Development Configuration
```cfg
server {
    port = 8080;
    buffer_size = 2048;           # Smaller buffer for development
    thread_pool_size = 8;         # Fewer threads for debugging
    keep_alive_timeout = 2;       # Shorter timeout for testing
    cluster_thread = 1;           # Single worker process
    max_connections = 1000;       # Lower connection limit
    accept_queue_size = 1024;     # Smaller queue
    accept_retry_delay_ms = 20;   # Slower retry
    max_events = 1024;            # Fewer events
}
```

#### Production Configuration
```cfg
server {
    port = 80;                    # Standard HTTP port
    buffer_size = 16384;          # Large buffer for production
    thread_pool_size = 128;       # High thread count
    keep_alive_timeout = 30;      # Long keep-alive
    cluster_thread = 8;           # Multiple worker processes
    max_connections = 2000000;    # Very high connection limit
    accept_queue_size = 262144;   # Large accept queue
    accept_retry_delay_ms = 1;    # Fast retry
    max_events = 32768;           # High event limit
}
```

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
- `const FormData &Request::getFormData(const std::string &key) const`: Returns multipart form field or file data.
- `bool File::save(const std::string &path) const`: Saves the uploaded file to the specified path.
- `std::string Request::getHeader(const std::string &key) const`: Gets request header value.
- `bool Request::isMultipartFormData() const`: Checks if request is multipart form data.

### Response Object

- `<< status << content`: Send response with status code and content
- `SendFile(path)`: Serve a file directly with MIME type detection
- `MovedRedirect(location)`: Send 301 permanent redirect
- `TemporaryRedirect(location)`: Send 302 temporary redirect
- `setHeader(key, value)`: Set custom response header
- `Render(view, data)`: Render a template with nlohmann::json data
- `std::string detectContentType(body)`: Automatically detect content type
- `void setStatus(code, message)`: Set custom status code and message
- `Response& setCookie(name, value, options)`: Set a cookie with options
- `std::optional<std::string> getCookie(name)`: Get cookie value
- `std::string getCookieValue(name, defaultValue)`: Get cookie with default
- `Response& setSignedCookie(name, value, secret, options)`: Set signed cookie
- `std::optional<std::string> getSignedCookie(name, secret)`: Get signed cookie
- `void removeCookie(name, path, domain, secure)`: Remove a cookie
- `void handleClient(clientSocket)`: Enhanced client handling with timeout and keep-alive
- `int initSocket(port, listenQueueSize)`: Optimized socket initialization
- `void acceptConnections()`: Epoll-based connection acceptance

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

## Development

### Development Setup

#### Prerequisites for Development
```bash
# Install development tools
sudo apt install build-essential cmake valgrind gdb

# Install additional development libraries
sudo apt install libboost-all-dev libcurl4-openssl-dev

# Install code formatting tools
sudo apt install clang-format clang-tidy
```

#### Development Workflow
```bash
# Clone and setup
git clone https://github.com/yourusername/nerva.git
cd nerva

# Create development branch
git checkout -b feature/new-feature

# Build with debug symbols
make clean
CXXFLAGS="-g -O0 -DDEBUG" make

# Run with debugger
gdb ./server

# Run with valgrind for memory checking
valgrind --leak-check=full --show-leak-kinds=all ./server
```

#### Code Style and Formatting
```bash
# Format code
find src/ includes/ -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# Check code style
find src/ includes/ -name "*.cpp" -o -name "*.hpp" | xargs clang-tidy
```

### Testing

#### Unit Testing Setup
```bash
# Install testing framework
sudo apt install libgtest-dev

# Build and run tests
make test
```

#### Integration Testing
```bash
# Install testing tools
sudo apt install curl wrk ab

# Run performance tests
wrk -t6 -c2000 -d10s http://localhost:8080/test

# Run Apache Bench tests
ab -n 10000 -c 100 http://localhost:8080/test
```

#### Manual Testing
```bash
# Test basic functionality
curl http://localhost:8080/
curl -X POST http://localhost:8080/json
curl http://localhost:8080/test/123

# Test file upload
curl -X POST -F "file=@test.txt" http://localhost:8080/upload

# Test authentication
curl "http://localhost:8080/protected?token=123"
```

## Troubleshooting

### Common Issues

#### Build Issues

**Problem**: Compilation fails with C++20 errors
```bash
error: 'concepts' is not a namespace-name
```
**Solution**: Ensure you have a C++20 compatible compiler
```bash
# Check compiler version
clang++ --version
# Should be clang++ 12+ or g++ 10+

# Update compiler if needed
sudo apt install clang-12
```

**Problem**: Missing simdjson library
```bash
fatal error: 'simdjson.h' file not found
```
**Solution**: Install simdjson development package
```bash
sudo apt install libsimdjson-dev
```

#### Runtime Issues

**Problem**: Server fails to start with "Address already in use"
```bash
bind: Address already in use
```
**Solution**: Check if port is already in use and kill the process
```bash
# Find process using port 8080
sudo lsof -i :8080

# Kill the process
sudo kill -9 <PID>

# Or use a different port in server.nrvcfg
```

**Problem**: High memory usage
```bash
# Monitor memory usage
htop
# or
ps aux | grep server
```
**Solution**: Adjust configuration parameters
```cfg
server {
    thread_pool_size = 16;        # Reduce thread count
    max_connections = 10000;      # Reduce connection limit
    buffer_size = 2048;           # Reduce buffer size
}
```

**Problem**: Server crashes with segmentation fault
```bash
# Run with gdb for debugging
gdb ./server
(gdb) run
# When crash occurs, check backtrace
(gdb) bt
```

#### Performance Issues

**Problem**: Low throughput
**Solution**: Optimize configuration
```cfg
server {
    thread_pool_size = 64;        # Increase threads
    buffer_size = 8192;           # Increase buffer
    keep_alive_timeout = 10;      # Increase keep-alive
}
```

**Problem**: High latency
**Solution**: Check system resources and optimize
```bash
# Monitor CPU and memory
top
iostat
vmstat

# Optimize system settings
echo 65535 > /proc/sys/net/core/somaxconn
echo 1 > /proc/sys/net/ipv4/tcp_tw_reuse
```

### Debugging

#### Enable Debug Logging
```cpp
// Add to your main.cpp
#define DEBUG_MODE 1
#include "Debug.hpp"

// Enable debug output
Debug::setLevel(DEBUG_LEVEL_VERBOSE);
```

#### Memory Leak Detection
```bash
# Run with valgrind
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./server

# Run with AddressSanitizer
CXXFLAGS="-fsanitize=address -g" make
```

#### Performance Profiling
```bash
# Install profiling tools
sudo apt install perf

# Profile the application
perf record ./server
perf report
```

### Logging and Monitoring

#### Enable Request Logging
```cpp
// Add logging middleware
Middleware loggingMiddleware = Middleware([](Http::Request &req, Http::Response &res, auto next) {
    std::cout << "[" << std::time(nullptr) << "] " 
              << req.method << " " << req.path 
              << " - " << res.statusCode << std::endl;
    next();
});

server.Use("/", {loggingMiddleware});
```

#### Monitor Server Health
```bash
# Check server status
curl -I http://localhost:8080/

# Monitor system resources
htop
iotop
netstat -tulpn | grep 8080
```

## Architecture

### System Architecture Overview

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Client        │    │   Load Balancer │    │   Nerva Server  │
│   (Browser/App) │◄──►│   (Optional)    │◄──►│   (Master)      │
└─────────────────┘    └─────────────────┘    └─────────┬───────┘
                                                       │ fork()
                                                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│  Worker Process │    │  Worker Process │    │  Worker Process │
│   (Thread Pool) │    │   (Thread Pool) │    │   (Thread Pool) │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

### Component Architecture

#### Request Flow
```
1. Client Request → 2. epoll Event → 3. Accept Thread → 4. Thread Pool → 5. Router → 6. Middleware → 7. Handler → 8. Response
```

#### Memory Management
- **tcmalloc**: High-performance memory allocator
- **Template Caching**: Compiled template storage
- **Direct Response Writing**: No intermediate buffers
- **Connection Pooling**: Reuse connections

### Security Considerations

#### Input Validation
```cpp
// Always validate user input
std::string sanitizeInput(const std::string& input) {
    // Remove potentially dangerous characters
    std::string sanitized = input;
    // Implementation details...
    return sanitized;
}
```

#### File Upload Security
```cpp
// Validate file uploads
server.Post("/upload", {}, [](const Http::Request &req, Http::Response &res) {
    auto fileData = req.getFormData("file");
    
    // Check file size
    if (fileData.file.size() > MAX_FILE_SIZE) {
        res << 413 << "File too large";
        return;
    }
    
    // Check file type
    if (!isAllowedFileType(fileData.filename)) {
        res << 400 << "File type not allowed";
        return;
    }
    
    // Save file safely
    std::string safePath = sanitizePath("./uploads/" + fileData.filename);
    fileData.file.save(safePath);
});
```

#### Authentication Best Practices
```cpp
// Use secure session management
Middleware secureAuth = Middleware([](Http::Request &req, Http::Response &res, auto next) {
    auto sessionId = res.getSignedCookie("session", SECRET_KEY);
    if (!sessionId || !isValidSession(*sessionId)) {
        res << 401 << "Unauthorized";
        return;
    }
    next();
});
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

We welcome contributions! Please follow these steps:

### Development Process

1. **Fork the repository**
   ```bash
   git clone https://github.com/yourusername/nerva.git
   cd nerva
   ```

2. **Create a feature branch**
   ```bash
   git checkout -b feature/amazing-feature
   ```

3. **Make your changes**
   - Follow the coding style guidelines
   - Add tests for new features
   - Update documentation

4. **Test your changes**
   ```bash
   make clean && make
   make test
   ```

5. **Submit a pull request**
   - Provide a clear description of changes
   - Include any relevant issue numbers
   - Ensure all tests pass

### Code Style Guidelines

- Use C++20 features appropriately
- Follow consistent naming conventions
- Add comments for complex logic
- Keep functions small and focused
- Use meaningful variable names

### Testing Guidelines

- Write unit tests for new features
- Ensure existing tests pass
- Add integration tests for API changes
- Test performance impact of changes

## Requirements

### System Requirements
- **Operating System**: Linux (uses epoll for event handling)
- **Architecture**: x86_64, ARM64
- **Memory**: Minimum 512MB RAM, recommended 2GB+
- **Storage**: 100MB free space for build artifacts

### Software Requirements
- **Compiler**: C++20 compatible (clang++ 12+ or g++ 10+)
- **Libraries**: 
  - simdjson (for high-performance JSON parsing)
  - nlohmann/json (for template engine data binding)
  - OpenSSL (for cryptographic functions)
  - tcmalloc (optional, for better performance)

### Development Requirements
- **Build Tools**: make, cmake
- **Debugging**: gdb, valgrind
- **Testing**: curl, wrk, ab
- **Code Quality**: clang-format, clang-tidy

---

**Made with ❤️ and ☕**

For more detailed documentation, see:
- [Architecture Documentation](ARCHITECTURE.md)
- [Template Engine Documentation](TEMPLATES.md)
- [API Reference](docs/API.md) 