#ifndef RADIXNODE_HPP
#define RADIXNODE_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <optional>

#include "Handlers.hpp"
#include "IHandler.hpp"

class RadixNode
{
public:
    explicit RadixNode(std::string segment = "");
    ~RadixNode();

    void insert(const std::vector<std::reference_wrapper<IHandler>> &middlewares, const std::string &method, const std::string &path, const RequestHandler &handler);
    std::optional<std::pair<RequestHandler, std::vector<std::reference_wrapper<IHandler>>>> find(const std::string &method, const std::string &path, std::map<std::string, std::string> &params) const;
    std::vector<RequestHandler> getAllHandlers(const std::string &method, const std::string &path) const;

private:
    std::string segment;
    std::vector<std::unique_ptr<RadixNode>> children;
    std::map<std::string, std::vector<RequestHandler>> methodHandlers;
    std::unordered_map<std::string, std::vector<std::reference_wrapper<IHandler>>> methodMiddlewares;

    bool isParam() const;
    RadixNode *findChild(const std::string &seg) const;
    RadixNode *findParamChild() const;

    static std::vector<std::string> split(const std::string &path);
};

#endif
