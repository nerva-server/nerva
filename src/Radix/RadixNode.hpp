#ifndef RADIXNODE_HPP
#define RADIXNODE_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>

class RadixNode
{
public:
    RadixNode(const std::string &segment = "");
    ~RadixNode();

    void addChild(std::unique_ptr<RadixNode> child);

    const std::string &getSegment() const;

private:
    std::string segment;
    std::vector<std::unique_ptr<RadixNode>> children;
};

#endif
