#pragma once
#include "src/dds/dds.h"

namespace dds {
    bool isComplexType(DdsDataType type);

    DdsSize sizeOfType(DdsDataType type);

    DdsSize std140Alignment(DdsDataType type);

    DdsSize cAlignment(DdsDataType type);

    DdsSize aline(DdsSize offset, DdsSize alignment);
}