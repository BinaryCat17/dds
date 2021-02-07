#include "components.hpp"

namespace dds {
    DefaultComponents makeDefaultComponents(InstanceData &data) {

    }

    DefaultComponents makeDefaultComponents(SerializeData &data, SerializeTablesData &tablesData) {
        return DefaultComponents{
                makeComponent(data.tables),
                makeComponent(data.columns),
                makeComponent(data.aosTables),
        };
    }

    AllocatorComponents makeAllocatorComponents(SerializeData &data, AllocatorData &allocatorData) {
        return AllocatorComponents{
                makeComponent(data.tables),
                Component{
                        data.columns.name,
                        data.columns.type,
                        data.columns.table,
                        data.columns.aosColumnOffset,
                        allocator.soaData,
                }, Component{
                        data.aosTables.table,
                        data.aosTables.rowSize,
                        allocator.aosData,
                },
        };
    }
}