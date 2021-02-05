#include "dds.h"
#include <filesystem>
#include <cista/serialization.h>
#include "src/dds/data/instance.hpp"
#include "src/dds/data/column.hpp"
#include "src/dds/data/table.hpp"
#include "src/dds/data/find.hpp"
#include "src/dds/helpers/TableListener.hpp"
#include "src/dds/helpers/OffsetIterator.hpp"

namespace fs = std::filesystem;

struct DdsInstanceT {
    dds::SerializeInfo info;
    dds::InstanceComponents components;
    std::unordered_map<DdsId, dds::TableListener> tableListeners{};
    dds::IdMaps idMaps{};
};

DdsResult ddsCreateInstance(DdsInstanceCreateFlags flags, const char *file, DdsInstance *pReturn) {
    if (!fs::exists(file)) {
        cista::buf buf{cista::mmap{file}};
        dds::InstanceData tmp{};
        cista::serialize(buf, tmp);
    }

    auto serializeInfo = dds::makeSerializeInfo(flags, file);
    auto components = dds::makeComponents(*serializeInfo.data);

    *pReturn = new DdsInstanceT{
        std::move(serializeInfo),
        std::move(components),
    };

    return DDS_RESULT_SUCCESS;
}

DdsResult ddsDeleteInstance(DdsInstance instance) {
    delete instance;
    return DDS_RESULT_SUCCESS;
}

DdsResult ddsSerialize(DdsInstance instance, DdsSerializeFlags) {
    cista::buf b{cista::mmap{instance->info.path.c_str(), cista::mmap::protection::WRITE}};
    cista::serialize(b, *instance->info.data);
    return DDS_RESULT_SUCCESS;
}

DdsResult ddsCreateTable(DdsInstance instance, DdsTableType type, char const *name,
        DdsSize columnCount, char const *const *pColumnNames, DdsDataType const *pColumnTypes,
        DdsId *pReturn) {
    auto &data = *instance->info.data;
    auto &components = instance->components;
    if (components.tableNameIndex[name]) {
        return DDS_RESULT_TABLE_ALREADY_EXIST;
    }

    DdsId table = components.tables.insert(name, 0);

    DdsResult result = dds::createColumns(components, table, columnCount, pColumnNames,
            pColumnTypes);

    if (result != DDS_RESULT_SUCCESS) {
        ddsDeleteTable(instance, table);
        return result;
    }

    if (type != DDS_TABLE_SOA) {
        DdsId aosId = components.aosTables.insert(table, dds::data::vector<uint8_t>{}, 0);
        result = dds::createAosColumns(data, components, table, aosId, type);
        if (result != DDS_RESULT_SUCCESS) {
            ddsDeleteTable(instance, table);
            return result;
        }
    }

    *pReturn = table;
    return DDS_RESULT_SUCCESS;
}

DdsResult ddsDeleteTable(DdsInstance instance, DdsId tableId) {
    auto &components = instance->components;
    components.tables.remove(tableId);
    return DDS_RESULT_SUCCESS;
}

DdsResult ddsFind(DdsInstance instance, DdsId column, DdsDataType type, void const *pValue,
        DdsId *pResult) {
    DdsColumnData data;
    DdsResult result = ddsColumnData(instance, column, type, &data);
    if (result != DDS_RESULT_SUCCESS) {
        return result;
    }

    return dds::getTypeMap(instance->idMaps, pValue, type,
            [data, column, instance, pResult](auto &map, auto value) {
                using value_type = std::decay_t<decltype(value)>;

                auto iter = map.find(column);
                if (iter == map.end()) {
                    auto &tableListener = instance->tableListeners[column];
                    dds::StrideIterator<value_type> begin(data.pData, data.stride);
                    dds::StrideIterator<value_type> end = begin + data.size / sizeof(value_type);
                    dds::Range range{begin, end};

                    iter = map.insert(std::pair{
                            column, dds::IdMap<value_type>(tableListener, range)}).first;
                }

                if (auto val = (iter->second)[value]) {
                    *pResult = *val;
                    return DDS_RESULT_SUCCESS;
                } else {
                    return DDS_RESULT_VALUE_NOT_EXIST;
                }
            });
}

DdsResult ddsGetTablesCount(DdsInstance instance, DdsSize *pReturn) {
    *pReturn = instance->info.data->tables.name.size();
    return DDS_RESULT_SUCCESS;
}

DdsResult ddsGetTable(DdsInstance instance, char const *name, DdsId *pReturn) {
    if (auto id = instance->components.tableNameIndex[name]) {
        *pReturn = *id;
        return DDS_RESULT_SUCCESS;
    } else {
        return DDS_RESULT_TABLE_NOT_EXIST;
    }
}

DdsResult ddsGetTableName(DdsInstance instance, DdsId table, char const **pReturn) {
    *pReturn = instance->info.data->tables.name[table].data();
    return DDS_RESULT_SUCCESS;
}

DdsResult ddsGetTableLength(DdsInstance instance, DdsId table, DdsSize *pReturn) {
    *pReturn = instance->info.data->tables.length[table];
    return DDS_RESULT_SUCCESS;
}

DdsResult ddsGetTableColumns(DdsInstance instance, DdsId table, DdsId const **pReturn,
        DdsSize *pColumnCount) {
    if (pReturn == nullptr) {
        *pColumnCount = instance->components.tableColumns[table].size();
    } else {
        *pReturn = instance->components.tableColumns[table].data();
    }
    return DDS_RESULT_SUCCESS;
}

DdsResult ddsGetColumn(DdsInstance instance, DdsId table, const char *name, DdsId *pReturn) {
    for (DdsSize column : instance->components.tableColumns[table]) {
        if (instance->info.data->columns.name[column] == name) {
            *pReturn = column;
            return DDS_RESULT_SUCCESS;
        }
    }
    return DDS_RESULT_COLUMN_NOT_EXIST;
}

DdsResult ddsGetColumnName(DdsInstance instance, DdsId column, char const **pReturn) {
    *pReturn = instance->info.data->columns.name[column].data();
    return DDS_RESULT_SUCCESS;
}

DdsResult ddsGetColumnType(DdsInstance instance, DdsId column, DdsDataType *pReturn) {
    *pReturn = instance->info.data->columns.type[column];
    return DDS_RESULT_SUCCESS;
}

DdsResult ddsInsert(DdsInstance instance, DdsId table, DdsSize count, DdsSize columnCount,
        DdsDataType const *pColumnTypes, DdsData const *pColumnData) {
    auto &data = *instance->info.data;
    auto &components = instance->components;

    if (components.tableColumns[table].size() != columnCount) {
        return DDS_RESULT_INVALID_TYPE;
    }

    DdsResult result = dds::checkColumns(data, components, table, count, pColumnTypes, pColumnData);
    if (result != DDS_RESULT_SUCCESS) {
        return result;
    }

    if (auto aosId = components.tableAosData[table]) {
        dds::aosInsert(components, data, table, *aosId, count, pColumnData);
    } else {
        dds::soaInsert(components, data, table, pColumnData);
    }

    instance->tableListeners[table].doInsert(count);

    return DDS_RESULT_SUCCESS;
}

DdsResult ddsRemove(DdsInstance instance, DdsId table, DdsId position) {
    auto &data = *instance->info.data;
    auto &components = instance->components;

    instance->tableListeners[table].doRemove(position);

    if (auto aosId = components.tableAosData[table]) {
        dds::aosRemove(data, *aosId, position);
    } else {
        dds::soaRemove(components, data, table, position);
    }
    return DDS_RESULT_SUCCESS;
}

DdsResult ddsColumnData(DdsInstance instance, DdsId column, DdsDataType type,
        DdsColumnData *pReturn) {
    auto &data = *instance->info.data;
    auto &components = instance->components;
    if (data.columns.type[column] != type) {
        return DDS_RESULT_INVALID_TYPE;
    }

    DdsId table = data.columns.table[column];

    if (auto aosId = components.tableAosData[table]) {
        *pReturn = dds::aosColumnData(data, *aosId, column);
    } else {
        *pReturn = dds::soaColumnData(data, column);
    }

    return DDS_RESULT_SUCCESS;
}
