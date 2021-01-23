#pragma once
#include <ranges>
#include <algorithm>

namespace ranges = std::ranges;
namespace views = std::views;

template<typename Cnt, typename T>
std::optional<DdsIndex> findIndex(Cnt const& c, T v) {
    auto iter = ranges::find(c, v);
    if(iter != c.end()) {
        return iter - c.begin();
    } else {
        return {};
    }
}

template<typename Cnt>
void unstableRemove(Cnt &c, size_t index) {
    auto iter = c.begin() + index;
    *iter = c.back();
    c.erase(c.end() - 1, c.end());
}

template<typename Map, typename Cnt>
void unstableRemoveWithAcceleration(Map& m, Cnt &c, size_t index) {
    m.at(c.back()) = index;
    m.erase(c[index]);
    unstableRemove(c, index);
}

template<typename T, typename Map, typename Cnt>
void pushWithAcceleration(T v, Map& m, Cnt &c) {
    c.push_back(v);
    m.emplace(v, c.size() - 1);
}

template<typename T, typename MapT>
std::optional<typename MapT::mapped_type> findInMapOpt(MapT const& c, T v) {
    auto iter = c.find(v);
    if(iter == c.end()) {
        return {};
    } else {
        return v.second;
    }
}
