#pragma once

#include <algorithm>
#include <cassert>

namespace dds {
    template<typename C, typename T>
    std::optional<size_t> findIndex(C const& c, T val) {
        auto iter = std::find(c.begin(), c.end(), val);
        if(iter != c.end()) {
            return static_cast<size_t>(iter - c.end());
        } else {
            return {};
        }
    }

// Move last value to removed variable. Return moved value index
    template<typename Cnt>
    void unstableRemove(Cnt &c, size_t index) {
        auto iter = c.data() + index;
        *iter = c.back();

        auto endIter = c.end();
        --endIter;
        c.erase(endIter);
    }

    template<typename Cnt, typename T>
    void unstableRemoveValue(Cnt &c, T value) {
        auto index = findIndex(c, value);
        assert(index && "value not found");
        unstableRemove(c, *index);
    }

}