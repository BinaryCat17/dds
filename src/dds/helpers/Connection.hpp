#pragma once
#include <cstdint>
#include <vector>
#include <optional>
#include "dds/helpers/generic.hpp"

namespace dds {
    class Connection {
    public:
        template<typename T1, typename C1, typename C2>
        explicit Connection(C1 &childConnection, T1 &childParentMember, C2 &parentConnection) {
            parentChild.resize(childParentMember.size(), notExist);
            for (size_t i = 0; i != childParentMember.size(); ++i) {
                parentChild[childParentMember[i]] = i;
            }

            childConnection.onInsert([this, &childParentMember](size_t count) {
                for (size_t i = childParentMember.size() - count; i != childParentMember.size(); ++i) {
                    parentChild[childParentMember[i]] = i;
                }
            });

            childConnection.onRemove([this, &childParentMember](size_t pos) {
                parentChild[childParentMember[pos]] = notExist;
                parentChild[childParentMember.back()] = pos;
            });

            parentConnection.onInsert([this](size_t count) {
                parentChild.resize(parentChild.size() + count, notExist);
            });

            parentConnection.onRemove([this, &childParentMember, &childConnection](size_t pos) {
                        childParentMember[parentChild.back()] = pos;
                        childConnection.remove(parentChild[pos]);
                        unstableRemove(parentChild, pos);
                    });
        }

        std::optional<size_t> operator[](size_t parent) const {
            if (parentChild[parent] != notExist) {
                return parentChild[parent];
            } else {
                return {};
            }
        }

    private:
        static const size_t notExist = std::numeric_limits<size_t>::max();
        std::vector<size_t> parentChild;
    };
}
