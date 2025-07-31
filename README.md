# Nerva HTTP Server

A high-performance, multi-threaded HTTP server written in C++20 with modern features including middleware support, static file serving, JSON handling, advanced template engine with nlohmann/json, and sophisticated routing capabilities.

## Features

- **High Performance**: Multi-threaded architecture with epoll-based event handling
- **Middleware Support**: Flexible middleware system for authentication and request processing
- **Static File Serving**: Built-in static file handler for serving public assets
- **JSON Support**: Integrated JSON parsing with nlohmann/json library
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
- **Custom 404 Pages**: Render custom not found pages with templates
- **Router Integration**: Modular API design with Router objects
- **Group Routing**: Route grouping for API versioning and modular structure
- **Memory Optimization**: tcmalloc integration for better memory management

## Quick Start

### Prerequisites

- C++20 compatible compiler (clang++ recommended)
- Linux system (uses epoll)
- nlohmann/json library
- tcmalloc (optional, for better performance)

### Building

```bash
make
```

### Running

```bash
make run
```

The server will start on port 8080 by default with tcmalloc preloaded for optimal performance.

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
- `Render(view, data)`: Render a template with nlohmann::json data

### Template Engine Features

- **Include System**: `{{ include templateName }}`
- **Conditionals**: `{{ if condition }}...{{ endif }}`
- **Loops**: `{{ for item in collection }}...{{ endfor }}`
- **Filters**: `{{ value|filter:param }}`
- **Data Binding**: Direct nlohmann::json object access

### JSON Utilities

- `Json::ParseAndReturnBody(jsonString)`: Parse and return JSON string
- `nlohmann::json`: Modern JSON library for data binding

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

## Template Engine Architecture

The Nerva template engine provides:

- **Modular Design**: Reusable template components (header, footer, cards)
- **Data Binding**: Seamless integration with nlohmann::json
- **Conditional Rendering**: Dynamic content based on data
- **Iterative Rendering**: Loop through collections and arrays
- **Custom Filters**: Extensible filter system for data transformation
- **Include System**: Template composition and reuse
- **CSS Integration**: Inline styling support in templates

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
- nlohmann/json library
- tcmalloc (optional, for better performance)

---

**Made with ❤️ and ☕** 