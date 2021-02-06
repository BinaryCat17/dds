#pragma once

#include "types.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t *(*DdsAllocate)(DdsSize size);

typedef void *(*DdsDeallocate)(uint8_t *position);

typedef struct DdsAllocator {
    DdsAllocate allocate;
    DdsDeallocate deallocate;
    void *pUserData;
} DdsAllocator;

#ifdef __cplusplus
}
#endif
