#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "NervaEngine.hpp"
#include <string>
#include <map>
#include <filesystem>

namespace Nerva
{

    class Engine : public TemplateEngine
    {
    public:
        void setViewsDirectory(const std::string &path);
        std::string render(const std::string &templateName, const Context &context) override;

    private:
        std::filesystem::path viewsDir;
        std::map<std::string, std::string> templateCache;

        std::string loadTemplate(const std::string &templateName);
        std::string renderString(const std::string &content, const Context &context);
        std::string processForLoop(const std::string &loopExpr, const std::string &loopContent, const Context &context);
        std::string processIfCondition(const std::string &ifExpr, const std::string &ifContent, const Context &context);
        std::string evaluateExpression(const std::string &expr, const Context &context);
        std::shared_ptr<Value> resolvePath(const std::string &path, const std::map<std::string, std::shared_ptr<Value>> &data);
        std::string getValueAsString(const std::shared_ptr<Value> &value);
    };

}

#endif