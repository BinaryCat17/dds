#include "InstanceData.hpp"
#include "utils.hpp"

// table

DdsIndex addTable(InstanceData &data, char const *name) {
    ++data.idCnt;
    pushWithAcceleration(data.idCnt, data.tableIdIndex, data.tableId);
    pushWithAcceleration(name, data.tableNameIndex, data.tableName);
    data.tableLength.push_back(0);
    data.tableRowSize.push_back(0);
    data.tableColumnIndices.push_back({});
    return data.tableId.size() - 1;
}

void removeTable(InstanceData &data, DdsIndex tableIndex) {
    unstableRemoveWithAcceleration(data.tableIdIndex, data.tableId, tableIndex);
    unstableRemoveWithAcceleration(data.tableNameIndex, data.tableName, tableIndex);
    unstableRemove(data.tableLength, tableIndex);
    unstableRemove(data.tableRowSize, tableIndex);
    unstableRemove(data.tableIdIndex, tableIndex);
    removeAosData(data, tableIndex);

    for (DdsIndex columnIndex : data.tableColumnIndices[tableIndex]) {
        removeColumn(data, columnIndex);
    }

    unstableRemove(data.tableColumnIndices, tableIndex);

    // correct indices after unstable remove
    for (DdsIndex index : data.tableColumnIndices.back()) {
        data.columnTableIndex[index] = tableIndex;
    }
}

DdsIndex addColumn(InstanceData &data, char const *name, DdsIndex tableIndex,
        DdsDataType type, uint32_t offset) {
    ++data.idCnt;
    pushWithAcceleration(data.idCnt, data.columnIdIndex, data.columnId);

    data.columnName.push_back(name);
    data.columnTableNameIndex.emplace(std::pair{tableIndex, name}, data.columnName.size() - 1);

    data.columnType.push_back(type);
    data.columnTableIndex.push_back(tableIndex);
    data.columnAosOffsets.push_back(offset);
    DdsIndex columnIndex = data.columnId.size() - 1;
    data.tableColumnIndices[tableIndex].push_back(columnIndex);
    return columnIndex;
}

void removeColumn(InstanceData &data, DdsIndex columnIndex) {
    unstableRemoveWithAcceleration(data.columnIdIndex, data.columnId, columnIndex);

    DdsId tableId = data.tableId[data.columnTableIndex[columnIndex]];
    data.columnTableNameIndex.at(std::pair{tableId, data.columnName.back()}) = columnIndex;
    data.columnTableNameIndex.erase(std::pair{tableId, data.columnName[columnIndex]});
    unstableRemove(data.columnName, columnIndex);

    unstableRemove(data.columnType, columnIndex);
    unstableRemove(data.columnTableIndex, columnIndex);
    unstableRemove(data.columnAosOffsets, columnIndex);

    auto &tableColumns = data.tableColumnIndices[columnIndex];
    unstableRemove(tableColumns, *findIndex(tableColumns, columnIndex));

    // correct indices after unstable remove
    DdsIndex lastColumn = data.columnId.size() - 1;
    auto &lastColumnTableIndices = data.tableColumnIndices[data.columnTableIndex[lastColumn]];
    *ranges::find(lastColumnTableIndices, lastColumn) = columnIndex;
}

void addSoaData(InstanceData &data, DdsIndex tableIndex, DdsIndex columnIndex) {
    data.soaColumnId.push_back(data.columnId[columnIndex]);
    size_t columnSize = data.tableLength[tableIndex] * sizeOfType(data.columnType[columnIndex]);
    data.soaData.emplace_back(data::vector<uint8_t>(columnSize));
}

void removeSoaData(InstanceData &data, DdsIndex soaIndex) {
    unstableRemove(data.soaColumnId, soaIndex);
    unstableRemove(data.soaData, soaIndex);
}

void addAosData(InstanceData &data, DdsIndex tableIndex, DdsTableType type) {
    data.aosTableId.push_back(data.tableId[tableIndex]);
    data.aosData.push_back({});
    data.aosPadding.push_back(0);
    data.aosType.push_back(type);
}

void removeAosData(InstanceData &data, DdsIndex aosIndex) {
    unstableRemove(data.aosTableId, aosIndex);
    unstableRemove(data.aosData, aosIndex);
    unstableRemove(data.aosPadding, aosIndex);
    unstableRemove(data.aosType, aosIndex);
}
