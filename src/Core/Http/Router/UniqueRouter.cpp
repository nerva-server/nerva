#include "Core/Http/Router/Router.hpp"

UniqueRouter::UniqueRouter( std::string request_type, Router *r) : request_type(request_type), r(r) {};

void UniqueRouter::Use(const std::string &path, std::vector<std::reference_wrapper<IHandler>> middlewares, const RequestHandler &handler) {
	r->addRoute(middlewares, request_type, path, handler);
}
