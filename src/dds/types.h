#pragma once
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t DdsId;
typedef uint64_t DdsSize;
typedef uint8_t DdsByte;

typedef struct DdsString16 {
    DdsSize length;
    char str[16];
} DdsString16;

typedef struct DdsString64 {
    DdsSize length;
    char str[64];
} DdsString64;

typedef struct DdsString256 {
    DdsSize length;
    char str[256];
} DdsString256;

typedef struct DdsVec2F {
    float x;
    float y;
} DdsVec2F;

typedef struct DdsVec3F {
    float x;
    float y;
    float z;
} DdsVec3F;

typedef struct DdsVec4F {
    float x;
    float y;
    float z;
    float w;
} DdsVec4F;

typedef float DdsMat3F[3][3];
typedef float DdsMat4F[4][4];

typedef struct DdsData {
    uint8_t const *pData;
    DdsSize size;
} DdsData;

typedef struct DdsColumnData {
    uint8_t *pData;
    DdsSize size;
    DdsSize stride;
} DdsColumnData;

#ifdef __cplusplus
}
#endif
