#pragma once

#include "types.h"
#include "allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DdsResult {
    DDS_RESULT_SUCCESS,
    DDS_RESULT_TABLE_NOT_EXIST,
    DDS_RESULT_COLUMN_NOT_EXIST,
    DDS_RESULT_VALUE_NOT_EXIST,
    DDS_RESULT_TABLE_ALREADY_EXIST,
    DDS_RESULT_INVALID_DATA,
    DDS_RESULT_INVALID_TYPE,
    DDS_RESULT_ALREADY_CONNECTED,
    DDS_RESULT_NOT_CONNECTED,
    DDS_RESULT_CHILD_NOT_EXIST,
} DdsResult;

typedef enum DdsTableType {
    DDS_TABLE_SOA, // structure of arrays
    DDS_TABLE_AOS, // array of structs
    DDS_TABLE_AOS_PACK, // without padding
    DDS_TABLE_AOS_STD140, // std140 uniform compatible
} DdsTableType;

typedef enum DdsConnectionType {
    DDS_CONNECTION_SINGLE,
    DDS_CONNECTION_MULTI,
} DdsConnectionType;

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

typedef struct DdsInstanceT DdsInstanceT;
typedef DdsInstanceT *DdsInstance;

typedef enum DdsInstanceCreateFlags {
    DDS_INSTANCE_CREATE_MMAP_WRITE = 0x00000001,
    DDS_INSTANCE_CREATE_MMAP_READ = 0x00000002,
} DdsInstanceCreateFlags;

typedef enum DdsSerializeFlags {
} DdsSerializeFlags;

DdsResult ddsCreateInstance(DdsInstanceCreateFlags flags, char const *file,
        DdsAllocator const *allocator, DdsInstance *pReturn);

DdsResult ddsDeleteInstance(DdsInstance instance);

DdsResult ddsSerialize(DdsInstance instance, DdsSerializeFlags flags);

DdsResult ddsCreateTable(DdsInstance instance, DdsTableType type, char const *name,
        DdsSize columnCount, char const *const *pColumnNames, DdsDataType const *pColumnTypes,
        DdsId *pReturn);

DdsResult ddsDeleteTable(DdsInstance instance, DdsId table);

DdsResult ddsFind(DdsInstance instance, DdsId column, DdsDataType type, void const *value,
        DdsId *pResult);

DdsResult ddsMakeConnection(DdsInstance instance, DdsId parentTable, DdsId childParentColumn,
        DdsConnectionType type);

DdsResult ddsFindChild(DdsInstance instance, DdsId childParentColumn, DdsId parentId,
        DdsId *pResult);

DdsResult ddsFindChildren(DdsInstance instance, DdsId childParentColumn, DdsId parentId,
        DdsId const **pResult, DdsSize *pChildrenCount);

DdsResult ddsGetTablesCount(DdsInstance instance, DdsSize *pReturn);

DdsResult ddsGetTable(DdsInstance instance, char const *name, DdsId *pReturn);

DdsResult ddsGetTableName(DdsInstance instance, DdsId table, char const **pReturn);

DdsResult ddsGetTableLength(DdsInstance instance, DdsId table, DdsSize *pReturn);

DdsResult ddsGetTableColumns(DdsInstance instance, DdsId table, DdsId const **pReturn,
        DdsSize *pColumnCount);

DdsResult ddsGetColumn(DdsInstance instance, DdsId table, const char *name, DdsId *pReturn);

DdsResult ddsGetColumnName(DdsInstance instance, DdsId column, char const **pReturn);

DdsResult ddsGetColumnType(DdsInstance instance, DdsId column, DdsDataType *pReturn);

DdsResult ddsInsert(DdsInstance instance, DdsId table, DdsSize count, DdsSize columnCount,
        DdsDataType const *pColumnTypes, DdsData const *pColumnData);

DdsResult ddsRemove(DdsInstance instance, DdsId table, DdsId position);

DdsResult ddsColumnData(DdsInstance instance, DdsId column, DdsDataType type,
        DdsColumnData *pResult);

DdsResult ddsAosData(DdsInstance instance, DdsId column, DdsData *pResult);

#ifdef __cplusplus
}
#endif
