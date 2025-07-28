#include "Router.hpp"
#include <iostream>

Router::Router() = default;

void Router::addRoute(const std::string &method, const std::string &path, const RequestHandler &handler)
{
    routes.insert(method, path, handler);
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
    std::map<std::string, std::string> params;
    auto handlerOpt = routes.find(req.method, req.path, params);

    if (handlerOpt.has_value())
    {
        handlerOpt.value()(req, res);
        return true;
    }

    res.setStatus(404, "Not Found");
    res.setHeader("Content-Type", "text/plain");
    res.body = "404 Not Found";
    return false;
}
