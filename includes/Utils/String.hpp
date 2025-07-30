#include <string>
#include <algorithm>

class String
{
public:
    static inline bool ends_with(const std::string &value, const std::string &ending)
    {
        if (ending.size() > value.size())
            return false;
        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    }

    static inline bool starts_with(const std::string &value, const std::string &start)
    {
        if (start.size() > value.size())
            return false;
        return std::equal(start.begin(), start.end(), value.begin());
    }
};
