#include "Router.hpp"
#include <iostream>
#include <sstream>

Router::Router()
{
}

void Router::addRoute(const std::string &method, const std::string &path, RequestHandler handler)
{
    routes[makeKey(method, path)] = handler;
}

void Router::Get(const std::string &path, const RequestHandler &handler)
{
    addRoute("GET", path, handler);
}

void Router::Post(const std::string &path, const RequestHandler &handler)
{
    addRoute("POST", path, handler);
}

bool Router::dispatch(const Http::Request &req, Http::Response &res) const
{
    auto it = routes.find(makeKey(req.method, req.path));
    if (it != routes.end())
    {
        it->second(req, res);
        return true;
    }

    res.setStatus(404, "Not Found");
    res.setHeader("Content-Type", "text/plain");
    res.body = "404 Not Found";
    return false;
}
