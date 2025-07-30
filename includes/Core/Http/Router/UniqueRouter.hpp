#include <string>

#include "Router.hpp"
#include "RouteBuilder.hpp"

class Router;

class UniqueRouter
{
	std::string request_type;
	Router *r;

public:
	UniqueRouter(std::string request_type, Router *r);

	void Use(const std::string &path, std::vector<std::reference_wrapper<IHandler>> middlewares, const RequestHandler &handler);
	RouteBuilder Register(const std::string &path);
};
