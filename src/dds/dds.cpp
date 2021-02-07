#include "dds.h"
#include <filesystem>
#include <cista/serialization.h>
#include "dds/data/instance.hpp"
#include "dds/data/column.hpp"
#include "dds/data/table.hpp"
#include "dds/data/search.hpp"
#include "dds/data/connection.hpp"
#include "dds/helpers/TableListener.hpp"
#include "dds/data/allocator.hpp"
#include "dds/data/helpers.hpp"
#include "dds/data/serialization.hpp"
#include "dds/data/components.hpp"

namespace fs = std::filesystem;

struct DdsInstanceT {
    dds::SerializeInfo info;
    dds::SearchHelpers components;
    std::unordered_map<DdsId, dds::TableListener> tableListeners{};
    dds::IdMaps idMaps{};
    dds::ColumnConnections connections{};
};

DdsResult ddsCreateInstance(DdsInstanceCreateFlags flags, const char *file,
        DdsAllocator const* allocator, DdsInstance *pReturn) {
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
    auto &data = *instance->info.data;
    auto &components = instance->components;

    auto f = [column, instance, pResult, &components, &data](auto &map, auto value) {
        using value_type = std::decay_t<decltype(value)>;

        auto iter = map.find(column);
        if (iter == map.end()) {
            auto &tableListener = instance->tableListeners[data.columns.table[column]];
            dds::getRange<value_type>(data, components, column,
                    [&tableListener, &iter, &map, column](auto range) {
                        dds::IdMap<value_type> idMap(tableListener, range);
                        return map.emplace(column, std::move(idMap)).first;
                    });
        }

        if (auto val = (iter->second)[value]) {
            *pResult = *val;
            return DDS_RESULT_SUCCESS;
        } else {
            return DDS_RESULT_VALUE_NOT_EXIST;
        }
    };

    return dds::getTypeMap(instance->idMaps, pValue, type, std::move(f));
}

DdsResult ddsMakeConnection(DdsInstance instance, DdsId parentTable, DdsId childParentColumn,
        DdsConnectionType type) {
    auto &data = *instance->info.data;
    auto &components = instance->components;

    DdsDataType dataType = data.columns.type[childParentColumn];

    auto &parentComponent = instance->tableListeners[parentTable];
    auto &childComponent = instance->tableListeners[data.columns.table[childParentColumn]];

    return dds::getTypeRange(data, dataType, components, childParentColumn,
            [childParentColumn, instance, type, &childComponent, &parentComponent](auto range) {
                return dds::insertConnection(instance->connections, childParentColumn, type,
                        childComponent, range, parentComponent);
            });
}

DdsResult ddsFindChild(DdsInstance instance, DdsId childParentColumn, DdsId parentId,
        DdsId *pResult) {
    auto &connections = instance->connections.single;
    auto iter = connections.find(childParentColumn);
    if (iter == connections.end()) {
        return DDS_RESULT_NOT_CONNECTED;
    }

    if (auto child = iter->second[parentId]) {
        *pResult = *child;
        return DDS_RESULT_SUCCESS;
    } else {
        return DDS_RESULT_CHILD_NOT_EXIST;
    }
}

DdsResult ddsFindChildren(DdsInstance instance, DdsId childParentColumn, DdsId parentId,
        DdsId const **pResult, DdsSize *pChildrenCount) {
    auto &connections = instance->connections.multi;
    auto iter = connections.find(childParentColumn);
    if (iter == connections.end()) {
        return DDS_RESULT_NOT_CONNECTED;
    }

    if (pResult == nullptr) {
        *pChildrenCount = iter->second[parentId].size();
    } else {
        *pResult = iter->second[parentId].data();
    }

    return DDS_RESULT_SUCCESS;
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

DdsResult ddsAosData(DdsInstance instance, DdsId column, DdsData *pResult) {
    auto &data = *instance->info.data;
    auto &components = instance->components;

    DdsId table = data.columns.table[column];

    if (auto aosId = components.tableAosData[table]) {
        auto &bytes = data.aosTables.data[*aosId];
        *pResult = DdsData{
                bytes.begin(),
                bytes.size(),
        };
        return DDS_RESULT_SUCCESS;
    } else {
        return DDS_RESULT_TABLE_NOT_EXIST;
    }
}
