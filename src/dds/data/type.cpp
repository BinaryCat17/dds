#include "type.hpp"

namespace dds {

    bool isComplexType(DdsDataType type) {
        switch (type) {
            case DDS_VEC2F_TYPE:
            case DDS_VEC3F_TYPE:
            case DDS_VEC4F_TYPE:
            case DDS_MAT3F_TYPE:
            case DDS_MAT4F_TYPE:
            case DDS_STRING16_TYPE:
            case DDS_STRING64_TYPE:
            case DDS_STRING256_TYPE:
                return true;
            default:
                return false;
        }
    }

    DdsSize sizeOfType(DdsDataType type) {
        switch (type) {
            case DDS_FLOAT_TYPE:
                return sizeof(float);
            case DDS_DOUBLE_TYPE:
                return sizeof(double);
            case DDS_INT32_TYPE:
                return sizeof(int32_t);
            case DDS_UINT32_TYPE:
                return sizeof(uint32_t);
            case DDS_INT64_TYPE:
                return sizeof(int64_t);
            case DDS_UINT64_TYPE:
                return sizeof(uint64_t);
            case DDS_STRING16_TYPE:
                return sizeof(DdsString16);
            case DDS_STRING64_TYPE:
                return sizeof(DdsString64);
            case DDS_STRING256_TYPE:
                return sizeof(DdsString256);
            case DDS_VEC2F_TYPE:
                return sizeof(DdsVec2F);
            case DDS_VEC3F_TYPE:
                return sizeof(DdsVec3F);
            case DDS_VEC4F_TYPE:
                return sizeof(DdsVec4F);
            case DDS_MAT3F_TYPE:
                return sizeof(DdsMat3F);
            case DDS_MAT4F_TYPE:
                return sizeof(DdsMat4F);
        }
        return 0;
    }

    DdsSize std140Alignment(DdsDataType type) {
        if (isComplexType(type)) {
            return 16;
        } else {
            return sizeOfType(type);
        }
    }

    DdsSize cAlignment(DdsDataType type) {
        if (isComplexType(type)) {
            return 0;
        } else {
            return sizeOfType(type);
        }
    }

    DdsSize aline(size_t offset, uint32_t alignment) {
        if (alignment) {
            return offset + (alignment - offset % alignment);
        } else {
            return offset;
        }
    }
}