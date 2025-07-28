#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>
#include <functional>
#include <map>
#include <sstream>

#include "Utils/Handlers.hpp"
#include "Radix/RadixNode.hpp"

class Router
{
public:
    Router();

    void addRoute(const std::string &method, const std::string &path, const RequestHandler &handler);

    void Get(const std::string &path, const RequestHandler &handler);
    void Post(const std::string &path, const RequestHandler &handler);

    bool dispatch(const Http::Request &req, Http::Response &res) const;

private:
    std::string makeKey(const std::string &method, const std::string &path) const
    {
        return method + ":" + path;
    }

    RadixNode routes;
};

#endif
