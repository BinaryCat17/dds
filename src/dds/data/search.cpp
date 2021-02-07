#include "search.hpp"

namespace dds {
    SearchHelpers makeSearchHelpers(InstanceData &data, Components &components) {
        return std::visit([&data](auto &&c) {
            return SearchHelpers{
                    IdMap{c.tables, data.tables.name},
                    MultiConnection{c.columns, data.columns.table, c.tables},
                    Connection{c.aosTables, data.aosTables.table, c.tables},
            };
        }, components);

    };
}