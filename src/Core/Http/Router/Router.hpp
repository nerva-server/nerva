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

class Router : public IHandler
{
public:
    Router();

    void addRoute(const std::string &method, const std::string &path, const RequestHandler &handler);

    void Get(const std::string &path, const RequestHandler &handler);
    void Post(const std::string &path, const RequestHandler &handler);
    void Put(const std::string &path, const RequestHandler &handler);
    void Delete(const std::string &path, const RequestHandler &handler);

    void Use(IHandler *handler)
    {
        if (handler)
        {
            handlers.push_back(std::unique_ptr<IHandler>(handler));
        }
    }

    bool dispatch(Http::Request &req, Http::Response &res) const;

    virtual void Handle(Http::Request& req, Http::Response& res, std::function<void()> next) override;

private:
    std::string makeKey(const std::string &method, const std::string &path) const
    {
        return method + ":" + path;
    }

    RadixNode routes;

    std::vector<std::unique_ptr<IHandler>> handlers;
};

#endif
