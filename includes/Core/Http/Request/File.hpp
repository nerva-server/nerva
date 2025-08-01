#ifndef FILE_HPP
#define FILE_HPP

#include <string>
#include <string_view>
#include <fstream>
#include <vector>
#include <memory>

namespace Http
{
    class File
    {
    public:
        File(std::string_view content) 
            : data_(content.data()), size_(content.size()), owned_data_() {}

        File(const char* data, size_t size) 
            : data_(data), size_(size), owned_data_() {}

        File(File&& other) noexcept
            : data_(other.data_), size_(other.size_), owned_data_(std::move(other.owned_data_))
        {
            other.data_ = nullptr;
            other.size_ = 0;
        }

        File& operator=(File&& other) noexcept
        {
            if (this != &other)
            {
                data_ = other.data_;
                size_ = other.size_;
                owned_data_ = std::move(other.owned_data_);
                
                other.data_ = nullptr;
                other.size_ = 0;
            }
            return *this;
        }

        File(const File& other)
        {
            if (other.owned_data_)
            {
                owned_data_ = std::make_shared<std::vector<char>>(*other.owned_data_);
                data_ = owned_data_->data();
                size_ = owned_data_->size();
            }
            else
            {
                owned_data_ = std::make_shared<std::vector<char>>(other.data_, other.data_ + other.size_);
                data_ = owned_data_->data();
                size_ = owned_data_->size();
            }
        }

        File& operator=(const File& other)
        {
            if (this != &other)
            {
                if (other.owned_data_)
                {
                    owned_data_ = std::make_shared<std::vector<char>>(*other.owned_data_);
                    data_ = owned_data_->data();
                    size_ = owned_data_->size();
                }
                else
                {
                    owned_data_ = std::make_shared<std::vector<char>>(other.data_, other.data_ + other.size_);
                    data_ = owned_data_->data();
                    size_ = owned_data_->size();
                }
            }
            return *this;
        }

        File() : data_(nullptr), size_(0), owned_data_() {}

        File(const std::string &content) 
            : owned_data_(std::make_shared<std::vector<char>>(content.begin(), content.end()))
        {
            data_ = owned_data_->data();
            size_ = owned_data_->size();
        }

        File(const std::vector<char> &data) 
            : owned_data_(std::make_shared<std::vector<char>>(data))
        {
            data_ = owned_data_->data();
            size_ = owned_data_->size();
        }

        ~File() = default;

        size_t size() const { return size_; }

        const char* data() const { return data_; }

        std::string_view view() const { return std::string_view(data_, size_); }

        std::vector<char> toVector() const 
        { 
            return std::vector<char>(data_, data_ + size_); 
        }

        std::string toString() const 
        { 
            return std::string(data_, size_); 
        }

        bool save(const std::string &path) const
        {
            if (!data_ || size_ == 0)
                return false;

            std::ofstream outFile(path, std::ios::binary);
            if (!outFile.is_open())
                return false;

            outFile.write(data_, size_);
            outFile.close();
            return true;
        }

        bool empty() const { return size_ == 0; }

        bool isOwned() const { return owned_data_ != nullptr; }

        void ensureOwned()
        {
            if (!owned_data_ && data_ && size_ > 0)
            {
                owned_data_ = std::make_shared<std::vector<char>>(data_, data_ + size_);
                data_ = owned_data_->data();
            }
        }

    private:
        const char* data_;
        size_t size_;
        std::shared_ptr<std::vector<char>> owned_data_;
    };
}

#endif