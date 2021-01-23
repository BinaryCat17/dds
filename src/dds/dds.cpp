#include "dds.h"
#include "utils.hpp"
#include "InstanceData.hpp"
#include <cista/serialization.h>
#include <cista/mmap.h>
#include <thread>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

using DataPtr = std::unique_ptr<InstanceData, void (*)(InstanceData *)>;

struct DdsInstanceT {
    std::string fileName;
    std::optional<cista::buf<cista::mmap>> mmapFile;
    std::unordered_multimap<DdsId, DdsReadCallback> columnReadCallbacks;
    std::unordered_multimap<DdsId, DdsListenCallback> tableInsertCallbacks;
    std::unordered_multimap<DdsId, DdsListenCallback> tableRemoveCallbacks;
    void *pUserData = nullptr;
    DataPtr data{nullptr, [](InstanceData *) {}};
};

DdsResult ddsCreateInstance(DdsInstanceCreateFlags flags, const char *file, DdsInstance *pRet) {
    if (!fs::exists(file)) {
        cista::buf buf{cista::mmap{file}};
        InstanceData tmp{};
        cista::serialize(buf, tmp);
    }

    *pRet = new DdsInstanceT{};
    DdsInstance instance = *pRet;
    instance->fileName = file;

    if (flags & DDS_INSTANCE_CREATE_MMAP) {
        instance->mmapFile = cista::buf{cista::mmap{file,
                cista::mmap::protection::MODIFY}};
        InstanceData *pData = data::deserialize<InstanceData>(*instance->mmapFile);
        instance->data = DataPtr(pData, [](InstanceData *) {});
    } else {
        cista::buf b{cista::mmap{file, cista::mmap::protection::READ}};
        InstanceData *pData = data::deserialize<InstanceData>(b);
        instance->data = DataPtr(new InstanceData(*pData), [](InstanceData *p) { delete p; });
    }

    return DDS_RESULT_SUCCESS;
}

DdsResult ddsDeleteInstance(DdsInstance instance) {
    delete instance;
    return DDS_RESULT_SUCCESS;
}

DdsResult ddsGetInstanceTables(DdsInstance instance, uint32_t *pCount, DdsId *pIds,
        char const **pNames) {
    auto &data = *instance->data;
    if (pCount != nullptr) {
        *pCount = data.tableId.size();
    } else {
        if (pIds != nullptr) {
            ranges::copy(data.tableId, pIds);
        }
        if (pNames != nullptr) {
            for (size_t i = 0; i != data.tableName.size(); ++i) {
                pNames[i] = data.tableName[i].data();
            }
        }
    }
    return DDS_RESULT_SUCCESS;
}

DdsResult ddsSerialize(DdsInstance instance, DdsSerializeFlags flags) {
    cista::buf b{cista::mmap{instance->fileName.data(), cista::mmap::protection::WRITE}};
    cista::serialize(b, *instance->data);
    return DDS_RESULT_SUCCESS;
}

DdsReturn ddsCreateTable(DdsInstance instance, DdsTableType type, const char *name) {
    auto &data = *instance->data;
    if (data.tableNameIndex.find(name) != data.tableNameIndex.end()) {
        return {DDS_RESULT_TABLE_ALREADY_EXIST, 0};
    } else {
        DdsIndex tableIndex = addTable(data, name);
        if (type != DDS_TABLE_CREATE_SOA) {
            addAosData(data, tableIndex, type);
        }
        return {DDS_RESULT_SUCCESS, data.tableId[tableIndex]};
    }
}

DdsResult ddsDeleteTable(DdsInstance instance, DdsId tableId) {
    auto &data = *instance->data;
    if (auto tableIndex = findInMapOpt(data.tableIdIndex, tableId)) {
        removeTable(data, *tableIndex);
        return DDS_RESULT_SUCCESS;
    } else {
        return DDS_RESULT_TABLE_NOT_EXIST;
    }
}

DdsReturn ddsGetTable(DdsInstance instance, char const *name) {
    auto &data = *instance->data;
    if (auto tableIndex = findInMapOpt(data.tableNameIndex, name)) {
        return {DDS_RESULT_SUCCESS, data.tableId[*tableIndex]};
    } else {
        return {DDS_RESULT_TABLE_NOT_EXIST, 0};
    }
}

DdsResult ddsGetTableColumns(DdsInstance instance, DdsId tableId, uint32_t *pCount,
        DdsId *pColumnIds, char const **pColumnNames, DdsDataType *pColumnTypes) {
    auto &data = *instance->data;
    if (auto tableIndex = findInMapOpt(data.tableIdIndex, tableId)) {
        if (pCount != nullptr) {
            *pCount = data.tableColumnIndices.size();
        } else {
            auto const &tableColumnIndices = data.tableColumnIndices[*tableIndex];
            if (pColumnIds != nullptr) {
                for (size_t i = 0; i != tableColumnIndices.size(); ++i) {
                    pColumnIds[i] = data.columnId[tableColumnIndices[i]];
                }
            }
            if (pColumnNames != nullptr) {
                for (size_t i = 0; i != tableColumnIndices.size(); ++i) {
                    pColumnNames[i] = data.columnName[tableColumnIndices[i]].data();
                }
            }
            if (pColumnTypes != nullptr) {
                for (size_t i = 0; i != tableColumnIndices.size(); ++i) {
                    pColumnTypes[i] = data.columnType[tableColumnIndices[i]];
                }
            }
        }
        return DDS_RESULT_SUCCESS;
    } else {
        return DDS_RESULT_TABLE_NOT_EXIST;
    }
}

size_t sizeOfType(DdsDataType type) {
    switch (type) {
        case DDS_FLOAT_TYPE:
            return sizeof(float);
        case DDS_DOUBLE_TYPE:
            return sizeof(double);
        case DDS_INT32_TYPE:
            return sizeof(int32_t);
        case DDS_UINT32_TYPE:
            return sizeof(uint32_t);
        case DDS_INT64_TYPE:
            return sizeof(int64_t);
        case DDS_UINT64_TYPE:
            return sizeof(uint64_t);
        case DDS_STRING16_TYPE:
            return sizeof(DdsString16);
        case DDS_STRING64_TYPE:
            return sizeof(DdsString64);
        case DDS_STRING256_TYPE:
            return sizeof(DdsString256);
        case DDS_VEC2F_TYPE:
            return sizeof(DdsVec2F);
        case DDS_VEC3F_TYPE:
            return sizeof(DdsVec3F);
        case DDS_VEC4F_TYPE:
            return sizeof(DdsVec4F);
        case DDS_MAT3F_TYPE:
            return sizeof(DdsMat3F);
        case DDS_MAT4F_TYPE:
            return sizeof(DdsMat4F);
    }
    throw std::runtime_error("incomplete switch");
}

uint32_t lastAlignment(uint32_t size) {
    uint32_t base = 16;
    while (!(size % base)) {
        base -= 4;
    }
    return base;
};

bool needAlignment(DdsDataType type) {
    switch (type) {
        case DDS_VEC2F_TYPE:
        case DDS_VEC3F_TYPE:
        case DDS_VEC4F_TYPE:
        case DDS_MAT3F_TYPE:
        case DDS_MAT4F_TYPE:
            return false;
        default:
            return true;
    }
};

DdsResult addAosDefaultColumn(uint32_t &rowSize, uint32_t &padding, DdsDataType type,
        uint32_t typeSize) {
    uint32_t alignment = std::max(lastAlignment(rowSize + padding), typeSize);
    if (needAlignment(type)) {
        uint32_t typeAlignment = (rowSize % typeSize);
        rowSize += typeAlignment - (rowSize & typeAlignment);
    }
    rowSize += typeSize;
    padding = alignment - (rowSize % alignment);

    return DDS_RESULT_SUCCESS;
}

uint32_t std140Alignment(DdsDataType type) {
    switch (type) {
        case DDS_VEC2F_TYPE:
        case DDS_VEC3F_TYPE:
        case DDS_VEC4F_TYPE:
        case DDS_MAT3F_TYPE:
        case DDS_MAT4F_TYPE:
            return 16;
        default:
            return sizeOfType(type);
    }
};

DdsResult addAosStd140Column(uint32_t &rowSize, DdsDataType type) {
    if (type == DDS_STRING16_TYPE || type == DDS_STRING64_TYPE ||
        type == DDS_STRING256_TYPE) {
        return DDS_RESULT_INVALID_TYPE;
    }
    uint32_t typeAlignment = std140Alignment(type);
    rowSize += typeAlignment - (rowSize & typeAlignment);

    return DDS_RESULT_SUCCESS;
}

DdsResult addAosColumn(InstanceData &data, DdsIndex aosIndex, DdsDataType type, uint32_t &rowSize,
        uint32_t typeSize) {
    switch (data.aosType[aosIndex]) {
        case DDS_TABLE_CREATE_AOS_PACK:
            rowSize += typeSize;
            return DDS_RESULT_SUCCESS;
        case DDS_TABLE_CREATE_AOS:
            return addAosDefaultColumn(rowSize, data.aosPadding[aosIndex], type, typeSize);
        case DDS_TABLE_CREATE_AOS_STD140:
            return addAosStd140Column(rowSize, type);
        default:
            return DDS_RESULT_UNDEFINED;
    }
}

DdsReturn ddsCreateColumn(DdsInstance instance, DdsDataType type, DdsId tableId, const char *name) {
    auto &data = *instance->data;
    if (auto tableIndex = findInMapOpt(data.tableIdIndex, tableId)) {
        if (findInMapOpt(data.columnTableNameIndex, std::pair{*tableIndex, name})) {
            return {DDS_RESULT_COLUMN_ALREADY_EXIST, 0};
        } else {
            auto &rowSize = data.tableRowSize[*tableIndex];
            DdsId columnId = addColumn(data, name, tableId, type, rowSize);
            uint32_t typeSize = sizeOfType(type);

            if (auto aosIndex = findIndex(data.aosTableId, tableId)) {
                return {addAosColumn(data, *aosIndex, type, rowSize, typeSize), columnId};
            } else {
                rowSize += typeSize;
                addSoaData(data, data.columnId.size() - 1, columnId);
                return {DDS_RESULT_SUCCESS, columnId};
            }
        }
    } else {
        return {DDS_RESULT_TABLE_NOT_EXIST, 0};
    }
}

DdsResult ddsDeleteColumn(DdsInstance instance, DdsId column) {
    auto &data = *instance->data;
    if (auto columnIndex = findIndex(data.columnId, column)) {
        removeColumn(data, *columnIndex);
    } else {
        return DDS_RESULT_COLUMN_NOT_EXIST;
    }
}

DdsReturn ddsGetColumn(DdsInstance instance, DdsId tableId, const char *name) {
    auto &data = *instance->data;
    if (auto tableIndex = findInMapOpt(data.tableIdIndex, tableId)) {
        std::pair key{*tableIndex, name};
        if (auto columnIndex = findInMapOpt(data.columnTableNameIndex, key)) {
            return {DDS_RESULT_SUCCESS, data.columnId[*columnIndex]};
        } else {
            return {DDS_RESULT_COLUMN_NOT_EXIST, 0};
        }
    } else {
        return {DDS_RESULT_TABLE_NOT_EXIST, 0};
    }
}

DdsResult tableInsert(DdsInstance instance, DdsIndex tableIndex, DdsIndex position, uint32_t count,
        uint32_t columnCount, DdsId const *pColumnIds, DdsDataType const *pColumnTypes,
        DdsData const *pColumnData) {
    auto &data = *instance->data;
    DdsId tableId = data.tableId[tableIndex];

    std::vector<size_t> columnIndices;
    for (size_t i = 0; i != columnCount; ++i) {
        if (auto columnIndex = findInMapOpt(data.tableIdIndex, pColumnIds[i])) {
            if (data.columnTableIndex[*columnIndex] != tableId) {
                return DDS_RESULT_COLUMN_NOT_EXIST;
            }
            columnIndices.push_back(*columnIndex);
            uint32_t typeSize = data.columnType[*columnIndex];
            if (typeSize != pColumnTypes[i]) {
                return DDS_RESULT_INVALID_TYPE;
            }

            size_t insertSize = typeSize * count;
            if (insertSize != pColumnData[i].size) {
                return DDS_RESULT_INVALID_DATA;
            }
        } else {
            return DDS_RESULT_COLUMN_NOT_EXIST;
        }
    }

    if (auto aosIndex = findIndex(data.aosTableId, tableId)) {
        size_t rowSize = data.tableRowSize[tableIndex] + data.aosPadding[*aosIndex];
        auto &bytes = data.aosData[*aosIndex];

        for (size_t j = 0; j != count; ++j) {
            for (size_t i = 0; i != columnCount; ++i) {
                size_t insertSize = sizeOfType(data.columnType[columnIndices[i]]);
                auto beg = reinterpret_cast<uint8_t const *>(pColumnData[i].pData) + insertSize * j;
                auto end = beg + insertSize;

                size_t offset = data.columnAosOffsets[columnIndices[i]];
                size_t tableOffset = (position + j) * rowSize;
                bytes.insert(bytes.begin() + tableOffset + offset, beg, end);
            }
        }
    } else {
        for (size_t i = 0; i != columnCount; ++i) {
            auto &bytes = data.soaData[*findIndex(data.soaColumnId, pColumnIds[i])];
            uint32_t typeSize = sizeOfType(data.columnType[columnIndices[i]]);
            size_t insertSize = typeSize * count;

            auto beg = reinterpret_cast<uint8_t const *>(pColumnData[i].pData);
            auto end = beg + insertSize;
            size_t offset = position * typeSize;
            bytes.insert(bytes.begin() + offset, beg, end);
        }
    }
    auto[beg, end] = instance->tableInsertCallbacks.equal_range(tableId);
    while (beg != end) {
        beg->second(position, instance->pUserData);
        ++beg;
    }
    ++data.tableLength[tableIndex];
    return DDS_RESULT_SUCCESS;
}

DdsResult tableRemove(DdsInstance instance, DdsIndex tableIndex, DdsIndex pos, uint32_t count) {
    auto &data = *instance->data;
    DdsId tableId = data.tableId[tableIndex];

    if (auto aosIndex = findIndex(data.aosTableId, tableId)) {
        auto &bytes = data.aosData[*aosIndex];
        uint32_t rowSize = data.tableRowSize[tableIndex] + data.aosPadding[*aosIndex];
        bytes.erase(bytes.begin() + pos * rowSize, bytes.begin() + (pos + count) * rowSize);
    } else {
        for (DdsId columnIndex : data.tableColumnIndices[tableIndex]) {
            size_t size = sizeOfType(data.columnType[columnIndex]);
            auto &bytes = data.soaData[*ranges::find(data.soaColumnId, data.columnId[columnIndex])];
            bytes.erase(bytes.begin() + pos * size, bytes.begin() + pos * (size + count));
        }
    }
    auto[beg, end] = instance->tableRemoveCallbacks.equal_range(tableId);
    while (beg != end) {
        beg->second(pos, instance->pUserData);
        ++beg;
    }
    --data.tableLength[tableIndex];
    return DDS_RESULT_SUCCESS;
}

DdsResult ddsPush(DdsInstance instance, DdsId table, uint32_t count, uint32_t columnCount,
        DdsId const *columnIds, DdsDataType const *columnTypes, DdsData const *columnData) {
    auto &data = *instance->data;
    if (auto tableIndex = findIndex(data.tableId, table)) {
        return tableInsert(instance, *tableIndex, data.tableLength[*tableIndex], count, columnCount,
                columnIds, columnTypes, columnData);
    } else {
        return DDS_RESULT_COLUMN_NOT_EXIST;
    }
}

DdsResult ddsPop(DdsInstance instance, DdsId table, uint32_t count) {
    auto &data = *instance->data;
    if (auto tableIndex = findIndex(data.tableId, table)) {
        return tableRemove(instance, *tableIndex, data.tableLength[*tableIndex] - count, count);
    } else {
        return DDS_RESULT_COLUMN_NOT_EXIST;
    }
}

DdsResult ddsInsert(DdsInstance instance, DdsId table, DdsIndex position, uint32_t count,
        uint32_t columnCount, DdsId const *columnIds, DdsDataType const *columnTypes,
        DdsData const *columnData) {
    auto &data = *instance->data;
    if (auto tableIndex = findIndex(data.tableId, table)) {
        return tableInsert(instance, *tableIndex, position, count, columnCount,
                columnIds, columnTypes, columnData);
    } else {
        return DDS_RESULT_COLUMN_NOT_EXIST;
    }
}

DdsResult ddsRemove(DdsInstance instance, DdsId table, DdsIndex position, uint32_t count) {
    auto &data = *instance->data;
    if (auto tableIndex = findIndex(data.tableId, table)) {
        return tableRemove(instance, *tableIndex, position, count);
    } else {
        return DDS_RESULT_COLUMN_NOT_EXIST;
    }
}


DdsResult ddsListen(DdsInstance instance, DdsId table, DdsListenCallback onInsert,
        DdsListenCallback onRemove) {
    instance->tableInsertCallbacks.emplace(table, onInsert);
    instance->tableRemoveCallbacks.emplace(table, onRemove);
}

DdsResult ddsReadColumn(DdsInstance instance, DdsId columnId, DdsDataType type,
        DdsReadCallback callback) {
    auto &data = *instance->data;
    if (auto columnIndex = findInMapOpt(data.columnIdIndex, columnId)) {
        if (data.columnType[*columnIndex] != type) {
            return DDS_RESULT_INVALID_TYPE;
        } else {
            instance->columnReadCallbacks.emplace(columnId, callback);
        }
    } else {
        return DDS_RESULT_COLUMN_NOT_EXIST;
    }
}

DdsResult ddsModifyColumn(DdsInstance instance, DdsId columnId, DdsDataType type,
        DdsModifyCallback callback) {
    auto &data = *instance->data;
    if (auto columnIndex = findInMapOpt(data.columnIdIndex, columnId)) {
        if (data.columnType[*columnIndex] != type) {
            return DDS_RESULT_INVALID_TYPE;
        } else {
            DdsIndex tableIndex = data.columnTableIndex[*columnIndex];
            DdsId tableId = data.tableId[tableIndex];
            if (auto aosIndex = findIndex(data.aosTableId, tableId)) {
                auto &bytes = data.aosData[*aosIndex];
                callback(bytes.data() + data.columnAosOffsets[*aosIndex], bytes.size(),
                        data.tableRowSize[tableIndex], instance->pUserData);
                auto[beg, end] = instance->columnReadCallbacks.equal_range(columnId);
                while (beg != end) {
                    beg->second(bytes.data() + data.columnAosOffsets[*aosIndex], bytes.size(),
                            data.tableRowSize[tableIndex], instance->pUserData);
                    ++beg;
                }
            } else {
                auto &bytes = data.soaData[*findIndex(data.soaColumnId, columnId)];
                callback(bytes.data(), bytes.size(),
                        sizeOfType(data.columnType[*columnIndex]), instance->pUserData);
                auto[beg, end] = instance->columnReadCallbacks.equal_range(columnId);
                while (beg != end) {
                    beg->second(bytes.data(), bytes.size(),
                            sizeOfType(data.columnType[*columnIndex]), instance->pUserData);
                    ++beg;
                }
            }
        }
    } else {
        return DDS_RESULT_COLUMN_NOT_EXIST;
    }
}
