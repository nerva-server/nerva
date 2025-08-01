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

class GroupBuilder
{
public:
    GroupBuilder(Router &router, std::string path);
    GroupBuilder &Use(IHandler &middleware);
    void Then(GroupHandler handler);

private:
    Router &router;
    std::string path;
    std::vector<std::reference_wrapper<IHandler>> middlewares;
};
