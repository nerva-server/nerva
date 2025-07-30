#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>
#include <functional>
#include <map>
#include <sstream>
#include <memory>
#include <vector>

#include "Utils/Handlers.hpp"
#include "Radix/RadixNode.hpp"
#include "Core/Http/Handler/IHandler.hpp"

#include "RouteBuilder.hpp"
#include "UniqueRouter.hpp"

class Router : public IHandler
{
public:
    Router();

    void addRoute(const std::vector<std::reference_wrapper<IHandler>> &middlewares, const std::string &method, const std::string &path, const RequestHandler &handler);

    void Get(const std::string &path, std::vector<std::reference_wrapper<IHandler>> middlewares, const RequestHandler &handler);
    void Post(const std::string &path, std::vector<std::reference_wrapper<IHandler>> middlewares, const RequestHandler &handler);
    void Put(const std::string &path, std::vector<std::reference_wrapper<IHandler>> middlewares, const RequestHandler &handler);
    void Delete(const std::string &path, std::vector<std::reference_wrapper<IHandler>> middlewares, const RequestHandler &handler);

    RouteBuilder Get(const std::string path);
    RouteBuilder Post(const std::string path);
    RouteBuilder Put(const std::string path);
    RouteBuilder Delete(const std::string path);

    void Use(const std::string &path, IHandler &handler)
    {
        handlers.push_back({path, std::unique_ptr<IHandler>(&handler)});
    }

    UniqueRouter operator[](std::string request_type)
    {
        return UniqueRouter{request_type, this};
    };

    bool dispatch(Http::Request &req, Http::Response &res, const std::string &basePath = "") const;

    virtual void Handle(Http::Request &req, Http::Response &res, std::function<void()> next) override;

private:
    std::string makeKey(const std::string &method, const std::string &path) const
    {
        return method + ":" + path;
    }

    bool tryDispatch(const std::string &fullPath, Http::Request &req, Http::Response &res) const;

    RadixNode routes;

    std::vector<std::pair<std::string, std::unique_ptr<IHandler>>> handlers;
};

#endif
