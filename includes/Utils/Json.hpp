#ifndef SRC_UTILS_JSON_HPP
#define SRC_UTILS_JSON_HPP

#include <string>
#include <simdjson.h>

class Json
{
public:
    static std::string ParseAndReturnBody(const std::string &jsonString)
    {
        simdjson::ondemand::parser parser;
        simdjson::padded_string padded = simdjson::padded_string(jsonString);
        simdjson::ondemand::document doc = parser.iterate(padded);

        std::ostringstream oss;
        oss << "{";

        bool first = true;
        for (auto field : doc.get_object())
        {
            if (!first)
                oss << ",";
            first = false;

            std::string_view key = field.unescaped_key();
            simdjson::ondemand::value val = field.value();

            oss << "\"" << key << "\":";

            switch (val.type())
            {
            case simdjson::ondemand::json_type::string:
                oss << "\"" << val.get_string() << "\"";
                break;
            case simdjson::ondemand::json_type::number:
                oss << val.get_double();
                break;
            case simdjson::ondemand::json_type::boolean:
                oss << (val.get_bool() ? "true" : "false");
                break;
            default:
                oss << "null";
            }
        }

        oss << "}";
        return oss.str();
    }

private:
    static simdjson::dom::parser parser;
    static simdjson::dom::element jsonElement;
};

#endif