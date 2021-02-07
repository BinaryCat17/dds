#pragma once

#include <functional>
#include <vector>
#include <cista/reflection/to_tuple.h>
#include <cstdint>
#include <cassert>
#include "Range.hpp"
#include "generic.hpp"

namespace dds {
    template<typename T>
    class Component {
    public:
        template<typename... Types>
        explicit Component(Types &&... v) : val(v...) {
            assert(check_valid());
        }

        template<typename... Types1>
        size_t insertRange(Types1 &&... args) {
            static_assert(sizeof...(Types1) == std::tuple_size_v<T>,
                    "insert elements count and structure members count are not equal");

            size_t count = insert_range_impl<0>(args...);
            for (auto const &f : insertCallbacks) {
                f(count);
            }
            return std::get<0>(val).size() - 1;
        }

        template<typename... Types1>
        size_t insert(Types1 &&... args) {
            static_assert(sizeof...(Types1) == std::tuple_size_v<T>,
                    "insert elements count and structure members count are not equal");

            insert_impl<0>(args...);
            for (auto const &f : insertCallbacks) {
                f(1);
            }
            return std::get<0>(val).size() - 1;
        }

        void remove(size_t pos) {
            for (auto const &f: removeCallbacks) {
                f(pos);
            }

            std::apply([pos](auto &&... args) {
                (dds::unstableRemove(args, pos), ...);
            }, val);
        }

        template<typename FnT>
        void onInsert(FnT && f) {
            insertCallbacks.emplace_back(f);
        }

        template<typename FnT>
        void onRemove(FnT && f) {
            removeCallbacks.emplace_back(f);
        }

    private:
        bool check_valid() const {
            size_t prevSize = std::get<0>(val).size();
            bool is_valid = true;
            auto check = [prevSize, &is_valid](auto &&m) {
                if (m.size() != prevSize) {
                    is_valid = false;
                }
            };
            std::apply([check](auto &&... args) { (check(args), ...); }, val);

            return is_valid;
        }

        template<size_t>
        size_t insert_range_impl() { return std::numeric_limits<size_t>::max(); }

        template<size_t pos, typename VT, typename... Types1>
        size_t insert_range_impl(VT &&v, Types1 &&... args) {
            using std::decay_t, std::tuple_element_t, std::is_convertible_v, std::get, std::forward;
            using cista::to_tuple;

            static_assert(is_convertible_v<typename decay_t<VT>::value_type,
                            typename decay_t<tuple_element_t<pos, decltype(val)>>::value_type>,
                    "insert element type and structure member type are not convertible");

            auto &elem = get<pos>(val);
            elem.insert(elem.end(), v.begin(), v.end());
            size_t size = insert_range_impl<pos + 1>(args...);
            assert((size == v.size() || size == std::numeric_limits<size_t>::max())
                   && "container sizes are not equal");
            return v.size();
        }

        template<size_t>
        void insert_impl() {}

        template<size_t pos, typename VT, typename... Types1>
        void insert_impl(VT &&v, Types1 &&... args) {
            using std::decay_t, std::tuple_element_t, std::is_convertible_v, std::get, std::forward;
            using cista::to_tuple;

            static_assert(is_convertible_v<decay_t<VT>,
                            typename decay_t<tuple_element_t<pos, decltype(val)>>::value_type>,
                    "insert element type and structure member type are not convertible");

            auto &elem = get<pos>(val);
            elem.push_back(v);
            insert_impl<pos + 1>(args...);
        }

        T val;
        std::vector<std::function<void(size_t count)>> insertCallbacks; // after insert
        std::vector<std::function<void(size_t pos)>> removeCallbacks; // below remove
    };

    template<typename... Types>
    Component(std::tuple<Types...>) -> Component<std::tuple<Types...>>;

    template<typename... Types>
    Component(Types...) -> Component<std::tuple<Types...>>;

    template<typename T>
    auto makeComponent(T &s) {
        return Component(cista::to_tuple(s));
    }

    template<typename T>
    struct ComponentTypeImpl {
        static inline T s;
        using type = decltype(makeComponent(s));
    };

    template<typename T>
    using StructComponentType = typename ComponentTypeImpl<T>::type;
    template<typename... Types>
    using ComponentType = Component<std::tuple<Types...>>;
}