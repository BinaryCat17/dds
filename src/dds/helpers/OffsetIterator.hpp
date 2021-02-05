#pragma once

#include <cstdint>
#include <iterator>

namespace dds {
    template<typename T>
    class StrideIterator {
        friend bool operator==(const StrideIterator &lhs, const StrideIterator &rhs) {
            return lhs.pData == rhs.pData &&
                   lhs.stride == rhs.stride;
        }

        friend bool operator!=(const StrideIterator &lhs, const StrideIterator &rhs) {
            return !(rhs == lhs);
        }

        friend StrideIterator operator+(StrideIterator lhs, size_t rhs) {
            return lhs += rhs;
        }

        friend StrideIterator operator-(StrideIterator lhs, size_t rhs) {
            return lhs -= rhs;
        }

        friend size_t operator-(StrideIterator lhs, StrideIterator rhs) {
            assert(lhs.stride == rhs.stride);
            return (reinterpret_cast<uint8_t *>(lhs.pData)
            - reinterpret_cast<uint8_t *>(rhs.pData)) / lhs.stride;
        }

    public:
        using iterator_category = std::random_access_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;  // or also value_type*
        using reference         = T&;

        explicit StrideIterator(void *pData, size_t stride) : pData(pData), stride(stride) {}

        T &operator*() const {
            return *(reinterpret_cast<T *>(pData));
        }

        StrideIterator &operator++() {
            pData = reinterpret_cast<uint8_t *>(pData) + stride;
            return *this;
        }

        StrideIterator operator++(int) {
            StrideIterator copy = *this;
            pData = reinterpret_cast<uint8_t *>(pData) + stride;
            return copy;
        }

        StrideIterator &operator--() {
            pData = reinterpret_cast<uint8_t *>(pData) - stride;
            return *this;
        }

        StrideIterator operator--(int) {
            StrideIterator copy = *this;
            pData = reinterpret_cast<uint8_t *>(pData) - stride;
            return copy;
        }

        StrideIterator &operator+=(size_t i) {
            pData = reinterpret_cast<uint8_t *>(pData) + stride * i;
            return *this;
        }

        StrideIterator &operator-=(size_t i) {
            pData = reinterpret_cast<uint8_t *>(pData) - stride * i;
            return *this;
        }

    private:
        void *pData;
        size_t stride;
    };
}
