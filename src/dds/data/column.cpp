#include "column.hpp"
#include "type.hpp"

namespace dds {
    DdsResult createColumns(InstanceHelpers &components, DdsId table,
            DdsSize columnCount, char const *const *pColumnNames, DdsDataType const *pColumnTypes) {
        for (size_t i = 0; i != columnCount; ++i) {
            components.columns.insert(pColumnNames[i], pColumnTypes[i], table, 0,
                    data::vector<uint8_t>{});
        }
        return DDS_RESULT_SUCCESS;
    }

    DdsResult addAosCStructColumn(InstanceData &data, InstanceHelpers &components, DdsId table,
            DdsId aosId) {
        auto &rowSize = data.aosTables.rowSize[aosId];
        auto const &columns = components.tableColumns[table];

        // C requires that structure size must be aligned at largest member size
        DdsSize rowSizeAlignment = 0;

        for (size_t i = 0; i != columns.size(); ++i) {
            DdsDataType type = data.columns.type[columns[i]];
            DdsSize typeSize = sizeOfType(type);

            data.columns.aosColumnOffset[columns[i]] = rowSize;
            rowSize = aline(rowSize, cAlignment(type));
            rowSize += typeSize;

            rowSizeAlignment = std::max(rowSizeAlignment, typeSize);
        }

        rowSize = aline(rowSize, rowSizeAlignment);
        return DDS_RESULT_SUCCESS;
    }

    DdsResult addAosStd140Column(InstanceData &data, InstanceHelpers &components, DdsId table,
            DdsId aosId) {
        auto &rowSize = data.aosTables.rowSize[aosId];
        auto const &columns = components.tableColumns[table];

        for (size_t i = 0; i != columns.size(); ++i) {
            DdsDataType type = data.columns.type[columns[i]];
            if (type == DDS_STRING16_TYPE ||
                type == DDS_STRING64_TYPE ||
                type == DDS_STRING256_TYPE) {
                return DDS_RESULT_INVALID_TYPE;
            }
            rowSize = aline(rowSize, std140Alignment(type));
        }

        return DDS_RESULT_SUCCESS;
    }

    DdsResult addAosPackColumn(InstanceData &data, InstanceHelpers &components, DdsId table,
            DdsId aosId) {
        auto &rowSize = data.aosTables.rowSize[aosId];
        auto const &columns = components.tableColumns[table];

        for (size_t i = 0; i != columns.size(); ++i) {
            rowSize += sizeOfType(data.columns.type[columns[i]]);
        }

        return DDS_RESULT_SUCCESS;
    }

    DdsResult createAosColumns(InstanceData &data, InstanceHelpers &components, DdsId table,
            DdsId aosId, DdsTableType tableType) {
        switch (tableType) {
            case DDS_TABLE_AOS_PACK:
                return addAosPackColumn(data, components, table, aosId);
            case DDS_TABLE_AOS:
                return addAosCStructColumn(data, components, table, aosId);
            case DDS_TABLE_AOS_STD140:
                return addAosStd140Column(data, components, table, aosId);
            default:
                return DDS_RESULT_INVALID_TYPE;
        }
    }

    DdsResult checkColumns(InstanceData &data, InstanceHelpers &components, DdsId table,
            DdsSize count, DdsDataType const *pColumnTypes, DdsData const *pColumnData) {
        for (size_t i = 0; i != components.tableColumns[table].size(); ++i) {
            DdsSize column = components.tableColumns[table][i];
            if (data.columns.type[column] != pColumnTypes[i]) {
                return DDS_RESULT_INVALID_TYPE;
            }
            if (dds::sizeOfType(data.columns.type[i]) * count != pColumnData[i].size) {
                return DDS_RESULT_INVALID_DATA;
            }
        }
    }

    DdsColumnData aosColumnData(InstanceData &data, DdsId aosId, DdsId column) {
        auto &bytes = data.aosTables.data[aosId];
        DdsSize offset = data.columns.aosColumnOffset[column];
        DdsSize rowSize = data.aosTables.rowSize[aosId];

        return DdsColumnData{
                bytes.data() + offset,
                bytes.size(),
                rowSize
        };
    }

    DdsColumnData soaColumnData(InstanceData &data, DdsId column) {
        auto &bytes = data.columns.soaColumnData[column];
        DdsSize typeSize = data.columns.type[column];

        return DdsColumnData{
                bytes.data(),
                bytes.size(),
                typeSize
        };
    }
}