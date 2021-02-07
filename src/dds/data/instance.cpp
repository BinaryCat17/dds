#include "instance.hpp"
#include <cista/serialization.h>

namespace dds {
    InstanceHelpers makeComponents(dds::InstanceData &data) {
        InstanceHelpers components = {
                makeComponent(data.tables),
                makeComponent(data.columns),
                makeComponent(data.aosTables),
                IdMap{components.tables, data.tables.name},
                MultiConnection{components.columns, data.columns.table, components.tables},
                Connection{components.aosTables, data.aosTables.table, components.tables},
        };
        return components;
    }

    SerializeInfo makeSerializeInfo(DdsInstanceCreateFlags flags, const char *file) {
        SerializeInfo info;
        info.path = file;

        if (flags & DDS_INSTANCE_CREATE_MMAP_WRITE) {
            info.mmap = cista::buf{cista::mmap{file, cista::mmap::protection::WRITE}};
            dds::InstanceData *pData = dds::data::deserialize<dds::InstanceData>(*info.mmap);
            info.data = DataPtr(pData, [](dds::InstanceData *) {});
        } else if (flags & DDS_INSTANCE_CREATE_MMAP_READ) {
            info.mmap = cista::buf{cista::mmap{file, cista::mmap::protection::READ}};
            dds::InstanceData *pData = dds::data::deserialize<dds::InstanceData>(*info.mmap);
            info.data = DataPtr(pData, [](dds::InstanceData *) {});
        } else {
            cista::buf b{cista::mmap{file, cista::mmap::protection::READ}};
            dds::InstanceData *pData = dds::data::deserialize<dds::InstanceData>(b);
            info.data = DataPtr(new dds::InstanceData(*pData),
                    [](dds::InstanceData *p) { delete p; });
        }

        return info;
    }

    std::pair<SerializeInfo, AllocatorData> makeAllocatorData(const char *file,
            DdsAllocator allocator) {
        AllocatorData allocatorData = {allocator, {}, {}};
        using DataVec = std::vector<uint8_t, DataAllocator<uint8_t>>;

        SerializeInfo info;
        info.path = file;

        cista::buf b{cista::mmap{file, cista::mmap::protection::READ}};
        dds::InstanceData *pData = dds::data::deserialize<dds::InstanceData>(b);
        for (auto const &tableData : pData->aosTables.data) {
            DataVec vec(tableData.begin(), tableData.end(), DataAllocator<uint8_t>(allocator));
            allocatorData.aosData.push_back(std::move(vec));
        }
        for (auto const &columnData : pData->columns.soaColumnData) {
            DataVec vec(columnData.begin(), columnData.end(), DataAllocator<uint8_t>(allocator));
            allocatorData.aosData.push_back(std::move(vec));
        }

        info.data = DataPtr(new dds::InstanceData(), [](dds::InstanceData *p) { delete p; });
        auto &data = *info.data;
        data.columns = {
                pData->columns.name,
                pData->columns.type,
                pData->columns.table,
                pData->columns.aosColumnOffset,
        };
        data.tables = pData->tables;
        data.aosTables = {
                pData->aosTables.table,
                pData->aosTables.rowSize,
        };
        return {std::move(info), std::move(allocatorData)};
    }

    InstanceHelpers makeAllocatorComponents(dds::InstanceData &data,
            dds::AllocatorData &allocator) {
        InstanceHelpers components = {
                makeComponent(data.tables),
                Component{data.columns.name,
                        data.columns.type,
                        data.columns.table,
                        data.columns.aosColumnOffset,
                        allocator.soaData,
                },
                makeComponent(data.aosTables),
                IdMap{components.tables, data.tables.name},
                MultiConnection{components.columns, data.columns.table, components.tables},
                Connection{components.aosTables, data.aosTables.table, components.tables},
        };
        return components;
    }
}
