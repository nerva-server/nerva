#include "Router.hpp"
#include <iostream>

Router::Router()
{
}

void Router::addRoute(const std::string &path, RequestHandler handler)
{
    routes[path] = handler;
}

std::string Router::dispatch(const std::string &requestPath) const
{
    auto it = routes.find(requestPath);
    if (it != routes.end())
    {
        return it->second(requestPath);
    }

    return "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\n404 Not Found";
}
