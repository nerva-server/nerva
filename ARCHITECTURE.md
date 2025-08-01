# Nerva HTTP Server - Architecture Documentation

## Overview

Nerva HTTP Server is a high-performance, multi-threaded HTTP server built with modern C++20. The architecture is designed for scalability, performance, and maintainability using event-driven I/O, thread pools, and process clustering.

## Core Architecture

### 1. Event-Driven Architecture

The server uses Linux epoll for efficient event-driven I/O handling:

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Client Socket │    │   epoll Event   │    │   Thread Pool   │
│   (Non-blocking)│◄──►│   Loop          │◄──►│   Worker        │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

**Key Components:**
- **epoll Event Loop**: Monitors multiple file descriptors for I/O events
- **Non-blocking Sockets**: All sockets are set to non-blocking mode
- **Event Notification**: epoll notifies when data is available for reading/writing

### 2. Multi-Process Clustering

The server implements a fork-based clustering system for load distribution:

```
┌─────────────────┐
│   Master Process│
│   (Main Process)│
└─────────┬───────┘
          │ fork()
          ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│  Worker Process │    │  Worker Process │    │  Worker Process │
│   (PID: 1001)   │    │   (PID: 1002)   │    │   (PID: 1003)   │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

**Process Management:**
- **Master Process**: Handles configuration, socket initialization, and worker management
- **Worker Processes**: Handle actual request processing
- **Shared Socket**: All workers share the same listening socket
- **Graceful Shutdown**: Proper signal handling and cleanup

### 3. Thread Pool Architecture

Each worker process uses a thread pool for concurrent request handling:

```
┌─────────────────┐
│  Accept Threads │
│  (4 threads)    │
└─────────┬───────┘
          │
┌─────────────────┐
│ Thread-Safe     │
│ Socket Queue    │
└─────────┬───────┘
          │
┌─────────────────┐
│ Worker Threads  │
│ (48 threads)    │
└─────────────────┘
```

**Thread Types:**
- **Accept Threads**: Handle new connection acceptance
- **Worker Threads**: Process HTTP requests
- **Thread-Safe Queue**: Synchronized socket distribution

## Component Architecture

### 1. Server Component

```cpp
class Server {
    ConfigParser config;           // Configuration management
    ThreadSafeQueue socketQueue;   // Socket distribution
    std::vector<std::thread> acceptThreads;
    std::vector<std::thread> threadPool;
    std::atomic<int> activeConnections;
    int serverSocket;
};
```

**Responsibilities:**
- Socket initialization and configuration
- Process and thread management
- Request routing and handling
- Configuration management

### 2. Router Component

```cpp
class Router {
    RadixTree routes;              // Route storage
    std::vector<HandlerPair> handlers;
    std::map<std::string, std::string> keys;
    Nerva::TemplateEngine* _engine;
};
```

**Features:**
- **Radix Tree Routing**: Fast route matching with O(k) complexity
- **Dynamic Parameters**: Route parameter extraction (`/users/:id`)
- **Middleware Support**: Chainable middleware functions
- **Route Groups**: Modular route organization

### 3. Request/Response Pipeline

```
HTTP Request → Request Parser → Router → Middleware → Handler → Response Builder → HTTP Response
```

**Request Processing:**
1. **Raw Request Parsing**: Parse HTTP headers and body
2. **Route Matching**: Find matching route using Radix tree
3. **Parameter Extraction**: Extract dynamic route parameters
4. **Middleware Execution**: Execute middleware chain
5. **Handler Execution**: Execute route handler
6. **Response Building**: Build HTTP response

### 4. Template Engine Architecture

```cpp
class Engine {
    std::string viewsDirectory;
    std::map<std::string, CompiledTemplate> templateCache;
    
    void render(Response& res, const std::string& templateName, const json& context);
    void compileTemplate(const std::string& templateName);
};
```

**Features:**
- **Template Caching**: Compiled templates for performance
- **JSON Data Binding**: Direct nlohmann::json integration
- **Include System**: Modular template composition
- **Conditionals and Loops**: Dynamic content rendering

## Data Flow

### 1. Connection Handling Flow

```
1. Client connects to server
2. epoll detects new connection
3. Accept thread accepts connection
4. Socket added to thread-safe queue
5. Worker thread picks up socket
6. Request parsing and processing
7. Response generation and sending
8. Connection cleanup
```

### 2. Request Processing Flow

```
1. Raw HTTP request received
2. Request parsing (headers, body, multipart)
3. Route matching using Radix tree
4. Parameter extraction
5. Middleware chain execution
6. Route handler execution
7. Response generation
8. HTTP response sent
```

### 3. Template Rendering Flow

```
1. Template name and context provided
2. Template loaded from cache or filesystem
3. Template compilation (if needed)
4. Context data binding
5. Template rendering with includes/loops/conditionals
6. HTML output generation
7. Response headers set
8. Response sent to client
```

## Performance Optimizations

### 1. Memory Management

- **tcmalloc Integration**: High-performance memory allocator
- **Template Caching**: Avoid repeated template compilation
- **String Optimization**: Minimize string copies
- **Direct Response Writing**: No intermediate buffers

### 2. I/O Optimizations

- **Non-blocking Sockets**: Asynchronous I/O operations
- **epoll Event Loop**: Efficient event notification
- **Connection Pooling**: Reuse connections with keep-alive
- **Buffer Management**: Configurable buffer sizes

### 3. Concurrency Optimizations

- **Thread Pool**: Reuse threads for request processing
- **Lock-free Queue**: Minimize synchronization overhead
- **Process Clustering**: Distribute load across processes
- **CPU Affinity**: Optimize thread placement

## Configuration Architecture

### 1. Configuration File Format

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

### 2. Configuration Management

```cpp
class ConfigParser {
    std::map<std::string, std::string> config;
    
    int getInt(const std::string& key);
    std::string getString(const std::string& key);
    bool getBool(const std::string& key);
};
```

## Security Architecture

### 1. Request Validation

- **Input Sanitization**: Validate and sanitize all inputs
- **Parameter Validation**: Type checking and bounds validation
- **Header Validation**: HTTP header integrity checks

### 2. Middleware Security

- **Authentication**: Token-based authentication
- **Authorization**: Role-based access control
- **Rate Limiting**: Request rate limiting capabilities

### 3. File Upload Security

- **File Type Validation**: MIME type checking
- **Size Limits**: Configurable file size limits
- **Path Validation**: Prevent directory traversal attacks

## Scalability Features

### 1. Horizontal Scaling

- **Process Clustering**: Multiple worker processes
- **Load Distribution**: Automatic load balancing
- **Graceful Scaling**: Dynamic process management

### 2. Vertical Scaling

- **Thread Pool**: Configurable thread count
- **Memory Optimization**: Efficient memory usage
- **CPU Optimization**: Multi-core utilization

### 3. Resource Management

- **Connection Limits**: Configurable connection limits
- **Memory Limits**: Memory usage monitoring
- **CPU Limits**: CPU usage optimization

## Monitoring and Debugging

### 1. Logging Architecture

- **Request Logging**: HTTP request/response logging
- **Error Logging**: Error tracking and reporting
- **Performance Logging**: Performance metrics collection

### 2. Metrics Collection

- **Connection Count**: Active connection monitoring
- **Request Rate**: Requests per second tracking
- **Response Time**: Response time measurement
- **Error Rate**: Error rate monitoring

### 3. Debugging Tools

- **Signal Handling**: Graceful shutdown and debugging
- **Process Monitoring**: Worker process health checks
- **Memory Profiling**: Memory usage analysis

## Future Architecture Considerations

### 1. Planned Enhancements

- **HTTP/2 Support**: Modern HTTP protocol support
- **WebSocket Support**: Real-time communication
- **SSL/TLS Integration**: Secure communication
- **API Gateway Features**: Advanced routing and load balancing

### 2. Performance Improvements

- **Zero-copy I/O**: Minimize data copying
- **Memory Mapping**: Efficient file serving
- **Compression**: Response compression
- **Caching**: Response caching mechanisms

### 3. Scalability Enhancements

- **Distributed Clustering**: Multi-server clustering
- **Service Discovery**: Dynamic service registration
- **Load Balancing**: Advanced load balancing algorithms
- **Auto-scaling**: Automatic resource scaling

---

This architecture documentation provides a comprehensive overview of the Nerva HTTP Server's internal design, components, and implementation details. The modular design allows for easy extension and maintenance while providing high performance and scalability. 