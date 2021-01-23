#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

// enums

typedef enum DdsResult {
    DDS_RESULT_UNDEFINED,
    DDS_RESULT_SUCCESS,
    DDS_RESULT_TABLE_NOT_EXIST,
    DDS_RESULT_COLUMN_NOT_EXIST,
    DDS_RESULT_TABLE_ALREADY_EXIST,
    DDS_RESULT_COLUMN_ALREADY_EXIST,
    DDS_RESULT_NEED_AOS,
    DDS_RESULT_NEED_SOA,
    DDS_RESULT_INVALID_DATA,
    DDS_RESULT_INVALID_TYPE,
} DdsResult;

typedef enum DdsTableType {
    DDS_TABLE_CREATE_SOA, // structure of arrays
    DDS_TABLE_CREATE_AOS, // array of structs
    DDS_TABLE_CREATE_AOS_PACK, // without padding
    DDS_TABLE_CREATE_AOS_STD140, // std140 uniform compatible
} DdsTableType;

typedef enum DdsDataType {
    DDS_STRING16_TYPE,
    DDS_STRING64_TYPE,
    DDS_STRING256_TYPE,
    DDS_FLOAT_TYPE,
    DDS_DOUBLE_TYPE,
    DDS_INT32_TYPE,
    DDS_UINT32_TYPE,
    DDS_INT64_TYPE,
    DDS_UINT64_TYPE,
    DDS_VEC2F_TYPE,
    DDS_VEC3F_TYPE,
    DDS_VEC4F_TYPE,
    DDS_MAT3F_TYPE,
    DDS_MAT4F_TYPE,
} DdsDataType;

// types

typedef struct DdsInstanceT DdsInstanceT;
typedef DdsInstanceT *DdsInstance;

typedef uint32_t DdsIndex;
typedef uint32_t DdsId;
typedef uint64_t DdsSize;

typedef struct DdsString16 {
    uint32_t length;
    char str[16];
} DdsString16;

typedef struct DdsString64 {
    uint32_t length;
    char str[64];
} DdsString64;

typedef struct DdsString256 {
    uint32_t length;
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
    void const *pData;
    uint64_t size;
} DdsData;

typedef struct DdsReturn {
    DdsResult result;
    DdsId value;
} DdsReturn;

// callbacks

typedef void(*DdsModifyCallback)(void *data, DdsSize size, DdsSize stride, void *userData);

typedef void(*DdsReadCallback)(void const *data, DdsSize size, DdsSize stride, void *userData);

typedef void(*DdsListenCallback)(DdsIndex position, void *userData);

// flags

typedef enum DdsInstanceCreateFlags {
    DDS_INSTANCE_CREATE_MMAP = 0x00000001,
} DdsInstanceCreateFlags;

typedef enum DdsSerializeFlags {
} DdsSerializeFlags;

// interface

DdsResult ddsCreateInstance(DdsInstanceCreateFlags flags, char const *file, DdsInstance *pRes);

DdsResult ddsDeleteInstance(DdsInstance instance);

DdsResult ddsGetInstanceTables(DdsInstance instance, uint32_t *pCount, DdsId *pIds,
        char const** pNames);

DdsResult ddsSerialize(DdsInstance instance, DdsSerializeFlags flags);

DdsReturn ddsCreateTable(DdsInstance instance, DdsTableType type, char const *name);

DdsResult ddsDeleteTable(DdsInstance instance, DdsId table);

DdsReturn ddsGetTable(DdsInstance instance, char const *name);

DdsResult ddsGetTableColumns(DdsInstance instance, DdsId table, uint32_t *pCount,
        DdsId *pColumnIds, char const **pColumnNames, DdsDataType *pColumnTypes);

DdsReturn ddsCreateColumn(DdsInstance instance, DdsDataType type, DdsId table, const char *name);

DdsResult ddsDeleteColumn(DdsInstance instance, DdsId column);

DdsReturn ddsGetColumn(DdsInstance instance, DdsId table, const char *name);

DdsResult ddsPush(DdsInstance instance, DdsId table, uint32_t count, uint32_t columnCount,
        DdsId const *columnIds, DdsDataType const *columnTypes, DdsData const *columnData);

DdsResult ddsPop(DdsInstance instance, DdsId table, uint32_t count);

DdsResult ddsInsert(DdsInstance instance, DdsId table, DdsIndex position, uint32_t count,
        uint32_t columnCount, DdsId const *columnIds, DdsDataType const *columnTypes,
        DdsData const *columnData);

DdsResult ddsRemove(DdsInstance instance, DdsId table, DdsIndex position, uint32_t count);

DdsResult ddsListen(DdsInstance instance, DdsId table, DdsListenCallback onInsert,
        DdsListenCallback onRemove);

DdsResult ddsReadColumn(DdsInstance instance, DdsId columnId, DdsDataType type,
        DdsReadCallback callback);

DdsResult ddsModifyColumn(DdsInstance instance, DdsId columnId, DdsDataType type,
        DdsModifyCallback callback);

#ifdef __cplusplus
}
#endif
