#pragma once

#include <cstdint>
#include <vector>

namespace dds {
    class MultiConnection {
    public:
        template<typename T1, typename C1, typename C2>
        explicit MultiConnection(C1 &childConnection, T1 &childParentMember, C2 &parentConnection) {
            parentChildren.resize(childParentMember.size());
            for (size_t i = 0; i != childParentMember.size(); ++i) {
                parentChildren[childParentMember[i]].push_back(i);
            }

            childConnection.onInsert([this, &childParentMember](size_t count) {
                for (size_t i = childParentMember.size() - count;
                     i != childParentMember.size(); ++i) {
                    parentChildren[childParentMember[i]].push_back(i);
                }
            });

            childConnection.onRemove([this, &childParentMember](size_t pos) {
                unstableRemoveValue(parentChildren[childParentMember[pos]], pos);
                auto &backChildren = parentChildren[childParentMember.back()];
                *std::find(backChildren.begin(), backChildren.end(),
                        childParentMember.size() - 1) = pos;
            });

            parentConnection.onInsert([this](size_t count) {
                parentChildren.resize(parentChildren.size() + count);
            });

            parentConnection.onRemove([this, &childParentMember, &childConnection](size_t pos) {
                for (size_t i = 0; i != parentChildren.size(); ++i) {
                    childParentMember[parentChildren.back()[i]] = pos;
                    childConnection.remove(parentChildren[pos][i]);
                    unstableRemove(parentChildren, pos);
                }
            });
        }

        std::vector<size_t> const &operator[](size_t parent) const {
            return parentChildren[parent];
        }

    private:
        std::vector<std::vector<size_t>> parentChildren;
    };
}
