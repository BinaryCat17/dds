#pragma once

#include "instance.hpp"

namespace dds {
    template<class T>
    class DataAllocator {
    public:
        friend bool operator==(const DataAllocator &lhs, const DataAllocator &rhs) {
            return lhs.alloc == rhs.alloc;
        }

        friend bool operator!=(const DataAllocator &lhs, const DataAllocator &rhs) {
            return !(rhs == lhs);
        }

    public:
        using value_type = T;

        DataAllocator(DdsAllocator alloc) : alloc(alloc) {};

        template<class U>
        constexpr DataAllocator(const DataAllocator<U> &r) noexcept : alloc(r.alloc) {}

        [[nodiscard]] T *allocate(std::size_t n) {
            alloc.allocate(n * sizeof(T));
        }

        void deallocate(T *p, std::size_t) noexcept {
            alloc.deallocate(reinterpret_cast<uint8_t*>(p));
        }

    private:
        DdsAllocator alloc;
    };



}
