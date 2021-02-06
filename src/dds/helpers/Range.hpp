#pragma once

#include <cstdint>
#include <iterator>
#include <vector>
#include "StrideIterator.hpp"

namespace dds {
    template<typename IterT>
    class Range {
    public:
        explicit Range(IterT first, IterT last) : first(first), last(last) {}

        using value_type = typename std::iterator_traits<IterT>::value_type;

        using iterator = IterT;

        IterT begin() const {
            return first;
        }

        IterT end() const {
            return last;
        }

        size_t size() const {
            return static_cast<size_t>(last - first);
        }

        value_type front() const {
            return *begin();
        }

        value_type& front() {
            return *begin();
        }

        value_type operator[](size_t id) const {
            return *(first + id);
        }

        value_type& operator[](size_t id){
            return *(first + id);
        }

        value_type back() const {
            return *(end() - 1);
        }

        value_type& back() {
            return *(end() - 1);
        }

    private:
        IterT first;
        IterT last;
    };

    template<typename T>
    class DataRange {
    public:
        explicit DataRange(std::vector<uint8_t> &data, size_t offset, size_t stride) :
                data(&data), offset(offset), stride(stride) {}

        using value_type = T;

        using iterator = T *;

        iterator begin() const {
            return StrideIterator<T>(data->begin() + offset, stride);
        }

        iterator end() const {
            return StrideIterator<T>(data->end() + offset, stride);
        }

        size_t size() const {
            return begin() - end();
        }

        value_type front() const {
            return *begin();
        }

        value_type &front() {
            return *begin();
        }

        value_type operator[](size_t id) const {
            return *(begin() + id);
        }

        value_type &operator[](size_t id) {
            return *(begin() + id);
        }

        value_type back() const {
            return *(end() - 1);
        }

        value_type &back() {
            return *(end() - 1);
        }

    private:
        std::vector<uint8_t>* data;
        size_t offset;
        size_t stride;
    };
}
