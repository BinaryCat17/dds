#pragma once
#include "dds/data/instance.hpp"

namespace dds {
    void aosInsert(InstanceHelpers &components, InstanceData &data, DdsId table, DdsId aosId,
            DdsSize count, DdsData const *pColumnData);

    void aosRemove(InstanceData &data, DdsId aosId, DdsId position);

    void soaInsert(InstanceHelpers &components, InstanceData &data, DdsId table,
            DdsData const *pColumnData);

    void soaRemove(InstanceHelpers &components, InstanceData &data, DdsId table,
            DdsId position);
}