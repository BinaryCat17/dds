#pragma once
#include <cstdint>

namespace dds {
    class TableListener {
    public:
        template<typename FnT>
        void onInsert(FnT && f) {
            insertCallbacks.emplace_back(f);
        }

        template<typename FnT>
        void onRemove(FnT && f) {
            removeCallbacks.emplace_back(f);
        }

        void doInsert(size_t count) {
            for(auto const& f : insertCallbacks) {
                f(count);
            }
        }

        void doRemove(size_t position) {
            for(auto const& f : removeCallbacks) {
                f(position);
            }
        }

    private:
        std::vector<std::function<void(size_t count)>> insertCallbacks; // after insert
        std::vector<std::function<void(size_t pos)>> removeCallbacks; // below remove
    };
}