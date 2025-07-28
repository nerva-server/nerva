#ifndef RADIXNODE_HPP
#define RADIXNODE_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <optional>

#include "Utils/Handlers.hpp"

class RadixNode
{
public:
    explicit RadixNode(std::string segment = "");
    ~RadixNode();

    void insert(const std::string &method, const std::string &path, const RequestHandler &handler);
    std::optional<RequestHandler> find(const std::string &method, const std::string &path, std::map<std::string, std::string> &params) const;

private:
    std::string segment;
    std::vector<std::unique_ptr<RadixNode>> children;
    std::map<std::string, RequestHandler> methodHandlers;

    bool isParam() const;
    RadixNode *findChild(const std::string &seg) const;
    RadixNode *findParamChild() const;

    static std::vector<std::string> split(const std::string &path);
};

#endif
