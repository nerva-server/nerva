# Nerva Template Engine - Documentation

## Overview

The Nerva Template Engine is a powerful, high-performance template system designed for dynamic HTML generation. It provides a clean, intuitive syntax with advanced features like includes, conditionals, loops, and custom filters, all while maintaining excellent performance through template caching and memory optimization.

## Features

- **Template Caching**: Compiled templates for faster rendering
- **JSON Data Binding**: Direct integration with nlohmann::json
- **Include System**: Modular template composition
- **Conditionals**: Dynamic content based on data
- **Loops**: Iterative content rendering
- **Custom Filters**: Extensible filter system
- **Memory Optimization**: Direct response writing without intermediate copies

## Basic Syntax

### 1. Variable Output

```html
<h1>{{ pageTitle }}</h1>
<p>Welcome, {{ user.name }}!</p>
<p>Price: ${{ product.price }}</p>
```

### 2. Object Property Access

```html
<div class="user-info">
    <p>Name: {{ user.profile.firstName }} {{ user.profile.lastName }}</p>
    <p>Email: {{ user.contact.email }}</p>
    <p>Phone: {{ user.contact.phone }}</p>
</div>
```

### 3. Array Access

```html
<ul>
    <li>{{ products[0].name }}</li>
    <li>{{ products[1].name }}</li>
    <li>{{ products[2].name }}</li>
</ul>
```

## Template Includes

### 1. Basic Include

```html
{{ include header }}
<main>
    <h1>Main Content</h1>
    <p>This is the main page content.</p>
</main>
{{ include footer }}
```

### 2. Include with Context

```html
{{ include productCard with product }}
{{ include userProfile with user }}
{{ include navigation with menuItems }}
```

### 3. Nested Includes

```html
{{ include header }}
<main>
    {{ include sidebar with sidebarData }}
    <div class="content">
        {{ include productList with products }}
    </div>
</main>
{{ include footer }}
```

## Conditional Rendering

### 1. Basic Conditionals

```html
{{ if user.isLoggedIn }}
    <p>Welcome back, {{ user.name }}!</p>
    <a href="/logout">Logout</a>
{{ endif }}

{{ if product.inStock }}
    <button class="buy-button">Add to Cart</button>
{{ endif }}
```

### 2. Complex Conditionals

```html
{{ if user.premium && product.discount > 0 }}
    <div class="premium-discount">
        <p>Premium Discount: {{ product.discount }}%</p>
    </div>
{{ endif }}

{{ if user.role == "admin" || user.role == "moderator" }}
    <div class="admin-panel">
        <a href="/admin">Admin Panel</a>
    </div>
{{ endif }}
```

### 3. Nested Conditionals

```html
{{ if user.isLoggedIn }}
    {{ if user.premium }}
        <div class="premium-features">
            <h3>Premium Features</h3>
            <ul>
                {{ for feature in user.premiumFeatures }}
                    <li>{{ feature }}</li>
                {{ endfor }}
            </ul>
        </div>
    {{ endif }}
{{ endif }}
```

## Loop Rendering

### 1. Basic Loops

```html
<ul class="product-list">
    {{ for product in products }}
        <li class="product-item">
            <h3>{{ product.name }}</h3>
            <p>Price: ${{ product.price }}</p>
        </li>
    {{ endfor }}
</ul>
```

### 2. Loop with Index

```html
<ol class="numbered-list">
    {{ for item, index in items }}
        <li>{{ index|add:1 }}. {{ item.name }}</li>
    {{ endfor }}
</ol>
```

### 3. Nested Loops

```html
<div class="categories">
    {{ for category in categories }}
        <div class="category">
            <h2>{{ category.name }}</h2>
            <ul class="products">
                {{ for product in category.products }}
                    <li>{{ product.name }} - ${{ product.price }}</li>
                {{ endfor }}
            </ul>
        </div>
    {{ endfor }}
</div>
```

## Custom Filters

### 1. Built-in Filters

```html
<!-- Format price -->
<p>Price: ${{ product.price|formatPrice }}</p>

<!-- Add numbers -->
<p>Item {{ index|add:1 }} of {{ totalItems }}</p>

<!-- String operations -->
<p>{{ product.name|uppercase }}</p>
<p>{{ product.description|truncate:100 }}</p>
```

### 2. Custom Filter Implementation

```cpp
// Register custom filters
engine->registerFilter("formatPrice", [](const std::string& value) {
    double price = std::stod(value);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << price;
    return oss.str();
});

engine->registerFilter("uppercase", [](const std::string& value) {
    std::string result = value;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
});
```

## Template Structure

### 1. Directory Structure

```
views/
├── layouts/
│   ├── base.html
│   └── admin.html
├── components/
│   ├── header.html
│   ├── footer.html
│   ├── navigation.html
│   └── productCard.html
├── pages/
│   ├── home.html
│   ├── products.html
│   ├── product-detail.html
│   └── notFound.html
└── partials/
    ├── sidebar.html
    └── pagination.html
```

### 2. Layout Templates

```html
<!-- layouts/base.html -->
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{{ pageTitle }}</title>
    <link rel="stylesheet" href="/static/css/style.css">
</head>
<body>
    {{ include header }}
    <main>
        {{ content }}
    </main>
    {{ include footer }}
    <script src="/static/js/app.js"></script>
</body>
</html>
```

### 3. Component Templates

```html
<!-- components/productCard.html -->
<div class="product-card">
    <img src="{{ product.image }}" alt="{{ product.name }}">
    <h3>{{ product.name }}</h3>
    <p class="price">${{ product.price|formatPrice }}</p>
    {{ if product.inStock }}
        <button class="add-to-cart">Add to Cart</button>
    {{ else }}
        <span class="out-of-stock">Out of Stock</span>
    {{ endif }}
</div>
```

## Data Binding

### 1. JSON Data Structure

```cpp
nlohmann::json data = {
    {"pageTitle", "Product Catalog"},
    {"user", {
        {"name", "John Doe"},
        {"email", "john@example.com"},
        {"premium", true},
        {"cartItems", 3}
    }},
    {"products", {
        {
            {"id", "1"},
            {"name", "Smartphone"},
            {"price", 799.99},
            {"inStock", true},
            {"image", "/static/images/phone.jpg"}
        },
        {
            {"id", "2"},
            {"name", "Laptop"},
            {"price", 1299.99},
            {"inStock", false},
            {"image", "/static/images/laptop.jpg"}
        }
    }},
    {"categories", {
        {"electronics", "Electronics"},
        {"clothing", "Clothing"},
        {"books", "Books"}
    }}
};
```

### 2. Template Usage

```cpp
server.Get("/products").Then([](const Http::Request &req, Http::Response &res) {
    // Prepare data
    nlohmann::json data = {
        {"pageTitle", "Product Catalog"},
        {"products", getProducts()},
        {"user", getCurrentUser()}
    };
    
    // Render template
    res.Render("products", data);
});
```

## Performance Features

### 1. Template Caching

```cpp
// Templates are automatically cached after first compilation
engine->setCacheEnabled(true);
engine->setCacheSize(1000); // Maximum cached templates
```

### 2. Memory Optimization

```cpp
// Direct response writing without intermediate string copies
void render(Http::Response &res, const std::string &templateName, const json &context);
```

### 3. Compilation Optimization

```cpp
// Templates are compiled once and reused
class CompiledTemplate {
    std::vector<TemplateToken> tokens;
    std::map<std::string, size_t> variableMap;
};
```

## Advanced Features

### 1. Template Inheritance

```html
<!-- base.html -->
<!DOCTYPE html>
<html>
<head>
    <title>{{ title }}</title>
    {{ block head }}{{ endblock }}
</head>
<body>
    {{ block content }}{{ endblock }}
</body>
</html>

<!-- page.html -->
{{ extends base }}
{{ block head }}
    <link rel="stylesheet" href="/custom.css">
{{ endblock }}
{{ block content }}
    <h1>{{ pageTitle }}</h1>
    <p>{{ content }}</p>
{{ endblock }}
```

### 2. Template Macros

```html
<!-- Define reusable macro -->
{{ macro button(text, class, url) }}
    <a href="{{ url }}" class="btn {{ class }}">{{ text }}</a>
{{ endmacro }}

<!-- Use macro -->
{{ button("Login", "btn-primary", "/login") }}
{{ button("Register", "btn-secondary", "/register") }}
```

### 3. Template Comments

```html
<!-- This is a regular HTML comment -->
{{# This is a template comment that won't appear in output #}}
{{ if debug }}
    {{# Debug information #}}
    <div class="debug-info">
        <p>User: {{ user.name }}</p>
        <p>Time: {{ currentTime }}</p>
    </div>
{{ endif }}
```

## Error Handling

### 1. Template Not Found

```cpp
// Graceful handling of missing templates
try {
    res.Render("missing-template", data);
} catch (const TemplateNotFound& e) {
    res << 500 << "Template not found: " << e.what();
}
```

### 2. Variable Not Found

```html
<!-- Safe variable access -->
<p>Welcome, {{ user.name|default:"Guest" }}!</p>
<p>Email: {{ user.email|default:"No email provided" }}</p>
```

### 3. Syntax Errors

```cpp
// Template compilation error handling
try {
    engine->compileTemplate("template.html");
} catch (const TemplateSyntaxError& e) {
    std::cerr << "Template syntax error: " << e.what() << std::endl;
}
```

## Best Practices

### 1. Template Organization

- Use descriptive template names
- Organize templates in logical directories
- Keep templates focused and reusable
- Use includes for common components

### 2. Performance Optimization

- Cache frequently used templates
- Minimize template complexity
- Use efficient data structures
- Avoid deep nesting in loops

### 3. Security Considerations

- Sanitize all user input
- Validate template data
- Use proper escaping for output
- Implement access controls

### 4. Maintainability

- Use consistent naming conventions
- Document complex templates
- Keep templates DRY (Don't Repeat Yourself)
- Use meaningful variable names

## Integration Examples

### 1. Express.js Style Routing

```cpp
server.Get("/users/:id").Then([](const Http::Request &req, Http::Response &res) {
    std::string userId = req.getParam("id");
    auto user = getUserById(userId);
    
    nlohmann::json data = {
        {"pageTitle", "User Profile"},
        {"user", user},
        {"isOwnProfile", user.id == getCurrentUserId()}
    };
    
    res.Render("user-profile", data);
});
```

### 2. API Response with Templates

```cpp
server.Get("/api/users/:id/html").Then([](const Http::Request &req, Http::Response &res) {
    std::string userId = req.getParam("id");
    auto user = getUserById(userId);
    
    nlohmann::json data = {
        {"user", user},
        {"includeAdminPanel", isAdmin()}
    };
    
    res.Render("user-card", data);
});
```

### 3. Dynamic Content Loading

```cpp
server.Get("/products/load-more").Then([](const Http::Request &req, Http::Response &res) {
    int page = std::stoi(req.getQuery("page"));
    auto products = getProductsByPage(page);
    
    nlohmann::json data = {
        {"products", products},
        {"hasMore", hasMoreProducts(page)}
    };
    
    res.Render("product-list-partial", data);
});
```

---

This template engine documentation provides comprehensive coverage of all features, syntax, and best practices for using the Nerva Template Engine effectively in your applications. 