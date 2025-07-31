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
        void render(Http::Response &res, const std::string &templateName, const json &context) override;

    private:
        std::filesystem::path viewsDir;
        std::map<std::string, std::string> templateCache;

        std::string loadTemplate(const std::string &templateName);
        std::string renderString(const std::string &content, const json &context);
        std::string processForLoop(const std::string &loopExpr,
                                   const std::string &loopContent,
                                   const json &context);
        std::string processIfCondition(const std::string &ifExpr,
                                       const std::string &ifContent,
                                       const json &context);
        std::string evaluateExpression(const std::string &expr,
                                       const json &context);
        json resolvePath(const std::string &path, const json &data);
    };
}

#endif