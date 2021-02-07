#pragma once

#include "instance.hpp"

namespace dds {
    DdsResult createColumns(InstanceHelpers &components, DdsId table, DdsSize columnCount,
            char const *const *pColumnNames, DdsDataType const *pColumnTypes);

    DdsResult createAosColumns(InstanceData &data, InstanceHelpers &components, DdsId table,
            DdsId aosId, DdsTableType tableType);

    DdsResult checkColumns(InstanceData &data, InstanceHelpers &components, DdsId table,
            DdsSize count, DdsDataType const *pColumnTypes, DdsData const *pColumnData);

    DdsColumnData aosColumnData(InstanceData &data, DdsId aosId, DdsId column);

    DdsColumnData soaColumnData(InstanceData &data, DdsId column);
}