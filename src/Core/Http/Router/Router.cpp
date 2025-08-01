#include "Router.hpp"

#include <iostream>

Router::Router()
{
    keys["views"] = "./views";
};

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

RouteBuilder Router::Get(const std::string path)
{
    return RouteBuilder(*this, "GET", path);
}

RouteBuilder Router::Post(const std::string path)
{
    return RouteBuilder(*this, "POST", path);
}

RouteBuilder Router::Put(const std::string path)
{
    return RouteBuilder(*this, "PUT", path);
}

RouteBuilder Router::Delete(const std::string path)
{
    return RouteBuilder(*this, "DELETE", path);
}

void Router::Set(std::string key, std::string value)
{
    keys[key] = value;
}

void Router::Set(std::string key, Nerva::TemplateEngine *value)
{
    _engine = value;
}

bool Router::tryDispatch(const std::string &fullPath, Http::Request &req, Http::Response &res) const
{
    std::map<std::string, std::string> params;

    auto result = routes.find(req.method, fullPath, params);

    if (!result.has_value())
    {
        result = routes.find(req.method, "/*", params);
    }

    if (result.has_value())
    {
        auto [firstHandler, middlewares] = result.value();

        for (const auto &[key, value] : params)
        {
            req.params[key] = value;
        }

        auto allHandlers = routes.getAllHandlers(req.method, fullPath);
        
        if (allHandlers.empty())
        {
            return false;
        }

        size_t handlerIndex = 0;
        size_t middlewareIndex = 0;
        
        std::function<void()> next = [&]()
        {
            if (middlewareIndex < middlewares.size())
            {
                auto &mw = middlewares[middlewareIndex++].get();
                mw.Handle(req, res, next);
            }
            else if (handlerIndex < allHandlers.size())
            {
                auto handler = allHandlers[handlerIndex++];
                handler(req, res, next);
            }
        };
        
        next();
        return true;
    }
    return false;
}

bool Router::dispatch(Http::Request &req, Http::Response &res, const std::string &basePath) const
{
    std::string fullPath = basePath + req.path;

    if (tryDispatch(fullPath, req, res))
    {
        return true;
    }

    if (!basePath.empty() && tryDispatch(req.path, req, res))
    {
        return true;
    }

    return false;
}

void Router::Handle(Http::Request &req, Http::Response &res, std::function<void()> next)
{
    size_t index = 0;
    std::function<void()> callNext = [&]()
    {
        if (index < handlers.size())
        {
            auto &handlerPair = handlers[index++];
            const std::string &handlerPath = handlerPair.first;
            IHandler *handler = handlerPair.second.get();

            if (req.path == handlerPath ||
                (req.path.length() > handlerPath.length() &&
                 req.path.substr(0, handlerPath.length()) == handlerPath &&
                 req.path[handlerPath.length()] == '/'))
            {
                std::string originalPath = req.path;
                req.path = req.path.substr(handlerPath.length());
                if (req.path.empty())
                    req.path = "/";

                handler->Handle(req, res, [&, originalPath]()
                                {
                    req.path = originalPath;
                    callNext(); });
            }
            else
            {
                callNext();
            }
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

void Router::Group(const std::string &path, std::vector<std::reference_wrapper<IHandler>> middlewares, GroupHandler handler)
{
    auto groupRouter = std::make_unique<Router>();
    handler(*groupRouter);

    auto &routerRef = *groupRouter;
    for (auto &middleware : middlewares)
    {
        routerRef.Use(path, middleware);
    }

    Use(path, std::move(groupRouter));
}

GroupBuilder Router::Group(std::string path)
{
    return GroupBuilder(*this, path);
}