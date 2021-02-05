#pragma once

#include <cstdint>
#include <unordered_map>
#include <optional>
#include <cassert>

namespace dds {
    template<typename T>
    class IdMap {
    public:
        IdMap() = delete;

        template<typename CT, typename T1>
        explicit IdMap(CT &connection, T1 & member) {
            for (size_t i = 0; i != member.size(); ++i) {
                map.emplace(member[i], i);
            }
            connection.onInsert([this, &member](size_t count) {
                for (size_t i = member.size() - count; i != member.size(); ++i) {
                    map.emplace(member[i], i);
                }
            });
            connection.onRemove([this, &member](size_t to) {
                auto iter = map.find(member.back());
                assert(iter != map.end() && "member no found");
                iter->second = to;
                map.erase(member[to]);
            });
        }

        std::optional<size_t> operator[](T const& v) const {
            auto iter = map.find(v);
            if (iter != map.end()) {
                return iter->second;
            } else {
                return {};
            }
        }

    private:
        std::unordered_map<T, size_t> map;
    };

    template<typename CT, typename T>
    IdMap(CT &, T &) -> IdMap<typename T::value_type>;
}