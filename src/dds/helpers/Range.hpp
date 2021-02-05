#pragma once

#include <cstdint>
#include <iterator>

namespace dds {
    template<typename IterT>
    class Range {
    public:
        explicit Range(IterT first, IterT last) : first(first), last(last) {}

        using value_type = typename std::iterator_traits<IterT>::value_type;

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
}
