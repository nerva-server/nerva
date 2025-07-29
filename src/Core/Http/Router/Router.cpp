#include "Core/Http/Router/Router.hpp"

#include <iostream>

Router::Router() = default;

void Router::addRoute(const std::vector<std::reference_wrapper<IHandler>> &middlewares, const std::string &method, const std::string &path, const RequestHandler &handler)
{
    routes.insert(middlewares, method, path, handler);
}

void Router::Get(const std::string &path, const std::vector<std::reference_wrapper<IHandler>> middlewares, const RequestHandler &handler)
{
    addRoute(middlewares, "GET", path, handler);
}

void Router::Post(const std::string &path, const std::vector<std::reference_wrapper<IHandler>> middlewares, const RequestHandler &handler)
{
    addRoute(middlewares, "POST", path, handler);
}

void Router::Put(const std::string &path, const std::vector<std::reference_wrapper<IHandler>> middlewares, const RequestHandler &handler)
{
    addRoute(middlewares, "PUT", path, handler);
}

void Router::Delete(const std::string &path, const std::vector<std::reference_wrapper<IHandler>> middlewares, const RequestHandler &handler)
{
    addRoute(middlewares, "DELETE", path, handler);
}

bool Router::dispatch(Http::Request &req, Http::Response &res) const
{
    std::map<std::string, std::string> params;
    auto result = routes.find(req.method, req.path, params);

    if (result.has_value())
    {
        auto [handler, middlewares] = result.value();

        for (const auto &[key, value] : params)
        {
            req.params[key] = value;
        }

        if (!middlewares.empty())
        {
            size_t current = 0;
            std::function<void()> next = [&]()
            {
                if (current < middlewares.size())
                {
                    auto &mw = middlewares[current++].get();
                    mw.Handle(req, res, next);
                }
                else
                {
                    handler(req, res); 
                }
            };
            next();
        }
        else
        {
            handler(req, res); 
        }

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