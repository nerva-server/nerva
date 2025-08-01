#ifndef FILE_HPP
#define FILE_HPP

#include <string>
#include <fstream>
#include <vector>

namespace Http
{
    class File
    {
    public:
        File() : data_(), size_(0) {}

        File(const std::string &content) : data_(content.begin(), content.end()), size_(content.size()) {}

        File(const std::vector<char> &data) : data_(data), size_(data.size()) {}

        size_t size() const { return size_; }

        const std::vector<char> &data() const { return data_; }

        bool save(const std::string &path) const
        {
            std::ofstream outFile(path, std::ios::binary);
            if (!outFile.is_open())
                return false;

            outFile.write(data_.data(), data_.size());
            outFile.close();
            return true;
        }

        bool empty() const { return size_ == 0; }

    private:
        std::vector<char> data_;
        size_t size_;
    };
}

#endif