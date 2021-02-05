#include "table.hpp"
#include "dds/data/type.hpp"


namespace dds {
    void aosInsert(InstanceComponents &components, InstanceData &data, DdsId table, DdsId aosId,
            DdsSize count, DdsData const *pColumnData) {
        auto &bytes = data.aosTables.data[aosId];
        DdsSize rowSize = data.aosTables.rowSize[aosId];
        bytes.resize(bytes.size() + count * rowSize);
        DdsSize currentSize = bytes.size();

        for (size_t i = 0; i != components.tableColumns[table].size(); ++i) {
            DdsSize column = components.tableColumns[table][i];
            DdsSize columnOffset = data.columns.aosColumnOffset[column];
            DdsSize typeSize = sizeOfType(data.columns.type[column]);
            for (size_t j = 0; j != count; ++j) {
                auto beg = pColumnData[i].pData + typeSize * j;
                std::copy(beg, beg + typeSize,
                        bytes.begin() + currentSize + j * rowSize + columnOffset);
            }
        }
    }

    void aosRemove(InstanceData &data, DdsId aosId, DdsId position) {
        auto &bytes = data.aosTables.data[aosId];
        uint32_t rowSize = data.aosTables.rowSize[aosId];
        auto beg = bytes.begin() + position * rowSize;
        std::copy(bytes.end() - rowSize, bytes.end(), beg);
        bytes.erase(bytes.end() - rowSize, bytes.end());
    }

    void soaInsert(InstanceComponents &components, InstanceData &data, DdsId table,
            DdsData const *pColumnData) {
        for (size_t i = 0; i != components.tableColumns[table].size(); ++i) {
            DdsSize column = components.tableColumns[table][i];

            auto &bytes = data.columns.soaColumnData[column];
            auto beg = pColumnData[i].pData;
            std::copy(beg, beg + pColumnData[i].size, std::back_inserter(bytes));
        }
    }

    void soaRemove(InstanceComponents &components, InstanceData &data, DdsId table,
            DdsId position) {
        for (size_t i = 0; i != components.tableColumns[table].size(); ++i) {
            DdsSize column = components.tableColumns[table][i];
            DdsSize typeSize = sizeOfType(data.columns.type[column]);

            auto &bytes = data.columns.soaColumnData[column];
            auto beg = bytes.begin() + position * typeSize;
            std::copy(bytes.end() - typeSize, bytes.end(), beg);
            bytes.erase(bytes.end() - typeSize, bytes.end());
        }
    }
}