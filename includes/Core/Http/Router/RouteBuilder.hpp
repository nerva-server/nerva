#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <functional>
#include <iostream>

#include "Handlers.hpp"
#include "IHandler.hpp"

class Router;

class RouteBuilder
{
public:
    RouteBuilder(Router &router, std::string method, std::string path);
    RouteBuilder &Use(IHandler &middleware);
    void Then(RequestHandler handler);

private:
    Router &router;
    std::string method, path;
    std::vector<std::reference_wrapper<IHandler>> middlewares;
};
