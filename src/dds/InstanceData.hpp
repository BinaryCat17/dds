#pragma once

#include "dds.h"
#include <cista/containers.h>

namespace data = cista::offset;

struct InstanceData {
    DdsId idCnt = 0;
    // table
    data::vector<DdsId> tableId{};
    data::vector<uint32_t> tableLength{};
    data::vector<uint32_t> tableRowSize{};
    data::vector<data::string> tableName{};
    data::vector<data::vector<DdsIndex>> tableColumnIndices{};
    data::hash_map<DdsId, DdsIndex> tableIdIndex{};
    data::hash_map<data::string, DdsId> tableNameIndex{};
    // column
    data::vector<DdsId> columnId{};
    data::vector<data::string> columnName{};
    data::vector<DdsDataType> columnType{};
    data::vector<DdsIndex> columnTableIndex{};
    data::vector<uint32_t> columnAosOffsets{};
    data::hash_map<DdsId, DdsIndex> columnIdIndex{};
    data::hash_map<std::pair<DdsId, data::string>, DdsIndex> columnTableNameIndex{};
    // soa
    data::vector<DdsId> soaColumnId{};
    data::vector<data::vector<uint8_t>> soaData{};
    // aos
    data::vector<DdsId> aosTableId{};
    data::vector<data::vector<uint8_t>> aosData{};
    data::vector<uint32_t> aosPadding{};
    data::vector<DdsTableType> aosType{};
};

// table

DdsIndex addTable(InstanceData &data, char const* name);

void removeTable(InstanceData &data, DdsIndex tableIndex);

// column

DdsIndex addColumn(InstanceData &data, char const* name, DdsIndex tableIndex,
        DdsDataType type, uint32_t offset);

void removeColumn(InstanceData &data, DdsIndex columnIndex);

// soa

void addSoaData(InstanceData &data, DdsIndex columnIndex, uint32_t offset);

void removeSoaData(InstanceData &data, DdsIndex columnIndex);

// aos

void addAosData(InstanceData &data, DdsIndex tableIndex, DdsTableType type);

void removeAosData(InstanceData &data, DdsIndex tableIndex);
