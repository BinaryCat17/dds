#pragma once
#include "dds/data/instance.hpp"

namespace dds {
    void aosInsert(InstanceComponents &components, InstanceData &data, DdsId table, DdsId aosId,
            DdsSize count, DdsData const *pColumnData);

    void aosRemove(InstanceData &data, DdsId aosId, DdsId position);

    void soaInsert(InstanceComponents &components, InstanceData &data, DdsId table,
            DdsData const *pColumnData);

    void soaRemove(InstanceComponents &components, InstanceData &data, DdsId table,
            DdsId position);
}