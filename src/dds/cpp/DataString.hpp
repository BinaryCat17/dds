#pragma once

#include <cstring>
#include <string>

namespace dds {
    template<uint32_t maxLen>
    class DataString {
    public:
        DataString() noexcept : length(0) {
            data[0] = '\0';
        }

        DataString(const DataString &rOther) noexcept : length(rOther.length) {
            length = length >= maxLen ? maxLen - 1 : length;
            memcpy(data, rOther.data, length);
            data[length] = '\0';
        }

        explicit DataString(const std::string &pString) noexcept :
                length((uint32_t) pString.length()) {
            length = length >= maxLen ? maxLen - 1 : length;
            memcpy(data, pString.c_str(), length);
            data[length] = '\0';
        }

        explicit DataString(char buf[maxLen]) noexcept : DataString((char const *) buf) {}

        DataString &operator=(const DataString &rOther) noexcept {
            if (this == &rOther) {
                return *this;
            }

            length = rOther.length;
            memcpy(data, rOther.data, length);
            data[length] = '\0';
            return *this;
        }

        DataString &operator=(const char *sz) noexcept {
            const int32_t len = (uint32_t)
                    ::strlen(sz);
            if (len > (int32_t) maxLen - 1) {
                return *this;
            }
            length = len;
            memcpy(data, sz, len);
            data[len] = 0;
            return *this;
        }

        DataString &operator=(const std::string &pString) noexcept {
            if (pString.length() > maxLen - 1) {
                return *this;
            }
            length = (uint32_t) pString.length();
            memcpy(data, pString.c_str(), length);
            data[length] = 0;
            return *this;
        }

        bool operator==(const DataString &other) const noexcept {
            return (length == other.length && 0 == memcmp(data, other.data, length));
        }

        bool operator!=(const DataString &other) const noexcept {
            return (length != other.length || 0 != memcmp(data, other.data, length));
        }

        DataString operator+=(const char *app) noexcept {
            const auto len = (uint32_t) ::strlen(app);
            if (!len) {
                return *this;
            }
            if (length + len >= maxLen) {
                return *this;
            }

            memcpy(&data[length], app, len + 1);
            length += len;
        }

        DataString operator+=(std::string const &str) noexcept {
            return *this += str.c_str();
        }

        void clear() noexcept {
            length = 0;
            data[0] = '\0';
        }

        char const *begin() const noexcept {
            return data;
        }

        char const *end() const noexcept {
            return data + length;
        }

        [[nodiscard]] const char *c_str() const noexcept {
            return data;
        }

        char *begin() noexcept {
            return data;
        }

        char *end() noexcept {
            return data + length;
        }

        size_t size() const noexcept {
            return length;
        }

    private:
        size_t length{};

        char data[maxLen]{};
    };

    using String16 = DataString<16>;
    using String64 = DataString<64>;
    using String256 = DataString<256>;
}

namespace std {
    template<>
    struct hash<dds::String16> {
        std::size_t operator()(dds::String16 const &s) const noexcept {
            return std::hash<std::string_view>{}(std::string_view{s.begin(), s.size()});
        }
    };

    template<>
    struct hash<dds::String64> {
        std::size_t operator()(dds::String64 const &s) const noexcept {
            return std::hash<std::string_view>{}(std::string_view{s.begin(), s.size()});
        }
    };

    template<>
    struct hash<dds::String256> {
        std::size_t operator()(dds::String256 const &s) const noexcept {
            return std::hash<std::string_view>{}(std::string_view{s.begin(), s.size()});
        }
    };
}