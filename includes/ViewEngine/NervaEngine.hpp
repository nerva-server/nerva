#ifndef NERVA_ENGINE_HPP
#define NERVA_ENGINE_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <memory>

namespace Nerva
{
    using json = nlohmann::json;

    class TemplateEngine
    {
    public:
        virtual ~TemplateEngine() = default;
        virtual std::string render(const std::string &templateName, const json &context) = 0;
    };
}

#endif