#pragma once

#include "instance.hpp"
#include "dds/helpers/Component.hpp"
#include <variant>

namespace dds {
    struct DefaultComponents {
        StructComponentType<TableData> tables;
        ComponentType<
                decltype(ColumnData::name),
                decltype(ColumnData::type),
                decltype(ColumnData::table),
                decltype(ColumnData::aosColumnOffset),
                decltype(SerializeTablesData::columnAosData)
        > columns;
        ComponentType<
                decltype(AosTableData::table),
                decltype(AosTableData::rowSize),
                decltype(SerializeTablesData::soaTableData)
        > aosTables;
    };

    struct AllocatorComponents {
        StructComponentType<TableData> tables;
        ComponentType<
                decltype(ColumnData::name),
                decltype(ColumnData::type),
                decltype(ColumnData::table),
                decltype(ColumnData::aosColumnOffset),
                decltype(AllocatorData::soaData)
        > columns;
        ComponentType<
                decltype(AosTableData::table),
                decltype(AosTableData::rowSize),
                decltype(AllocatorData::aosData)
        > aosTables;
    };

    using Components = std::variant<DefaultComponents, AllocatorComponents>;

    DefaultComponents makeDefaultComponents(SerializeData &data, SerializeTablesData &tablesData);

    AllocatorComponents makeAllocatorComponents(SerializeData &data, AllocatorData &allocatorData);
}
