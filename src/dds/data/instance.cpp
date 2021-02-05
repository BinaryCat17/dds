#include "instance.hpp"
#include <cista/serialization.h>

namespace dds {
    InstanceComponents makeComponents(dds::InstanceData &data) {
        InstanceComponents components = {
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
}
