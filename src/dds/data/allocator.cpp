#include "allocator.hpp"

namespace dds {
    AllocatorData makeAllocatorData(InstanceData &data, DdsAllocator allocator) {
        using DataVec = std::vector<uint8_t, DataAllocator<uint8_t>>;
        DataAllocator<uint8_t> stlAlloc(allocator);
        AllocatorData allocatorData = { allocator, {}, {} };

        for (auto const &tableData : data.aosTables.data) {
            DataVec vec(tableData.begin(), tableData.end(), stlAlloc);
            allocatorData.aosData.push_back(std::move(vec));
        }
        for (auto const &columnData : data.columns.soaColumnData) {
            DataVec vec(columnData.begin(), columnData.end(), stlAlloc);
            allocatorData.aosData.push_back(std::move(vec));
        }

        return allocatorData;
    }
}