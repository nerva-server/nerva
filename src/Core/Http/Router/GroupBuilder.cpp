#include <functional>

#include "GroupBuilder.hpp"
#include "Router.hpp"
#include "Middleware.hpp"

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

    auto& routerRef = *groupRouter;
    for(auto &middleware : middlewares) {
        routerRef.Use(path, middleware);
    }

    router.Use(path, std::move(groupRouter));
}