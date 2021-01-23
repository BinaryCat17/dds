#pragma once

#include <cstring>
#include <string>

namespace dds {
    template<uint32_t maxLen>
    struct DataString {
        DataString() : length(0) {
            data[0] = '\0';
        }

        DataString(const DataString &rOther) : length(rOther.length) {
            length = length >= maxLen ? maxLen - 1 : length;
            memcpy(data, rOther.data, length);
            data[length] = '\0';
        }

        explicit DataString(const std::string &pString) :
                length((uint32_t) pString.length()) {
            length = length >= maxLen ? maxLen - 1 : length;
            memcpy(data, pString.c_str(), length);
            data[length] = '\0';
        }

        explicit DataString(char buf[maxLen]) : DataString((char const*)buf) {}

        void set(const std::string &pString) {
            if (pString.length() > maxLen - 1) {
                return;
            }
            length = (uint32_t) pString.length();
            memcpy(data, pString.c_str(), length);
            data[length] = 0;
        }

        void set(const char *sz) {
            const int32_t len = (uint32_t)
            ::strlen(sz);
            if (len > (int32_t) maxLen - 1) {
                return;
            }
            length = len;
            memcpy(data, sz, len);
            data[len] = 0;
        }

        DataString &operator=(const DataString &rOther) {
            if (this == &rOther) {
                return *this;
            }

            length = rOther.length;
            memcpy(data, rOther.data, length);
            data[length] = '\0';
            return *this;
        }

        DataString &operator=(const char *sz) {
            set(sz);
            return *this;
        }

        DataString &operator=(const std::string &pString) {
            set(pString);
            return *this;
        }

        bool operator==(const DataString &other) const {
            return (length == other.length && 0 == memcmp(data, other.data, length));
        }

        bool operator!=(const DataString &other) const {
            return (length != other.length || 0 != memcmp(data, other.data, length));
        }

        void append(const char *app) {
            const auto len = (uint32_t) ::strlen(app);
            if (!len) {
                return;
            }
            if (length + len >= maxLen) {
                return;
            }

            memcpy(&data[length], app, len + 1);
            length += len;
        }

        void clear() {
            length = 0;
            data[0] = '\0';
        }

        [[nodiscard]] const char *c_str() const {
            return data;
        }

        uint32_t length{};

        char data[maxLen]{};
    };

    using String16 = DataString<16>;
    using String64 = DataString<64>;
    using String256 = DataString<256>;
}