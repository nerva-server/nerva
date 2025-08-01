#include <functional>

#include "RouteBuilder.hpp"
#include "Router.hpp"
#include "Middleware.hpp"

RouteBuilder::RouteBuilder(Router &router, std::string method, std::string path)
    : router(router), method(std::move(method)), path(std::move(path)) {}

RouteBuilder &RouteBuilder::Use(IHandler &middleware)
{
    middlewares.push_back(middleware);
    return *this;
}

void RouteBuilder::Then(RequestHandler handler)
{
    router.addRoute(middlewares, method, path, handler);
}