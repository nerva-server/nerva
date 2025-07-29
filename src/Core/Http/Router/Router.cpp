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

void Router::Put(const std::string &path, const RequestHandler &handler)
{
    addRoute("PUT", path, handler);
}

void Router::Delete(const std::string &path, const RequestHandler &handler)
{
    addRoute("DELETE", path, handler);
}

bool Router::dispatch(Http::Request &req, Http::Response &res) const {
    std::map<std::string, std::string> params;
    auto handlerOpt = routes.find(req.method, req.path, params);
    
    if (handlerOpt.has_value()) {
        for (const auto& [key, value] : params) {
            req.params[key] = value;
        }
        
        handlerOpt.value()(req, res);
        return true;
    }
    
    res.setStatus(404, "Not Found");
    return false;
}

void Router::Handle(Http::Request &req, Http::Response &res, std::function<void()> next)
{
    size_t index = 0;
    std::function<void()> callNext = [&]()
    {
        if (index < handlers.size())
        {
            handlers[index++]->Handle(req, res, callNext);
        }
        else
        {
            if (!dispatch(req, res))
            {
                next();
            }
        }
    };
    callNext();
}