#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <functional>
#include <iostream>

#include "Utils/Handlers.hpp"
#include "Core/Http/Handler/IHandler.hpp"

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
