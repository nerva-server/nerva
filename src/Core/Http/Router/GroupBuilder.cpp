#include <functional>

#include "Core/Http/Router/GroupBuilder.hpp"
#include "Core/Http/Router/Router.hpp"
#include "Core/Http/Middleware/Middleware.hpp"

GroupBuilder::GroupBuilder(Router &router, std::string path)
    : router(router), path(std::move(path)) {}

GroupBuilder &GroupBuilder::Use(IHandler &middleware)
{
    middlewares.push_back(middleware);
    return *this;
}

void GroupBuilder::Then(GroupHandler handler) {
    auto groupRouter = std::make_unique<Router>();
    
    handler(*groupRouter);
    
    router.Use(path, std::move(groupRouter));
}