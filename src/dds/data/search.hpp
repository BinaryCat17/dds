#pragma once

#include "instance.hpp"
#include "components.hpp"
#include "dds/helpers/IdMap.hpp"
#include "dds/cpp/DataString.hpp"

namespace dds {
    struct SearchHelpers {
        IdMap<data::string> tableNameIndex;
        MultiConnection tableColumns;
        Connection tableAosData;
    };

    SearchHelpers makeSearchHelpers(InstanceData &data, Components &components);

    struct IdMaps {
        std::unordered_map<DdsId, IdMap<dds::String16>> strings16;
        std::unordered_map<DdsId, IdMap<dds::String64>> strings64;
        std::unordered_map<DdsId, IdMap<dds::String256>> strings256;
        std::unordered_map<DdsId, IdMap<float>> floats;
        std::unordered_map<DdsId, IdMap<double>> doubles;
        std::unordered_map<DdsId, IdMap<uint64_t>> uints64;
        std::unordered_map<DdsId, IdMap<int64_t>> ints64;
        std::unordered_map<DdsId, IdMap<uint32_t>> uints32;
        std::unordered_map<DdsId, IdMap<int32_t>> ints32;
    };

    template<typename FnT>
    DdsResult getTypeMap(IdMaps &maps, void const *pData, DdsDataType type, FnT &&f) {
        switch (type) {
            case DDS_STRING16_TYPE:
                return f(maps.strings16, *reinterpret_cast<dds::String16 const *>(pData));
            case DDS_STRING64_TYPE:
                return f(maps.strings64, *reinterpret_cast<dds::String64 const *>(pData));
            case DDS_STRING256_TYPE:
                return f(maps.strings256, *reinterpret_cast<dds::String256 const *>(pData));
            case DDS_FLOAT_TYPE:
                return f(maps.floats, *reinterpret_cast<float const *>(pData));
            case DDS_DOUBLE_TYPE:
                return f(maps.doubles, *reinterpret_cast<double const *>(pData));
            case DDS_INT32_TYPE:
                return f(maps.ints32, *reinterpret_cast<int32_t const *>(pData));
            case DDS_UINT32_TYPE:
                return f(maps.uints32, *reinterpret_cast<uint32_t const *>(pData));
            case DDS_INT64_TYPE:
                return f(maps.ints64, *reinterpret_cast<int64_t const *>(pData));
            case DDS_UINT64_TYPE:
                return f(maps.uints64, *reinterpret_cast<uint64_t const *>(pData));
            case DDS_VEC2F_TYPE:
            case DDS_VEC3F_TYPE:
            case DDS_VEC4F_TYPE:
            case DDS_MAT3F_TYPE:
            case DDS_MAT4F_TYPE:
            default:
                return DDS_RESULT_INVALID_TYPE;
        }
    }

    template<typename T, typename FnT>
    DdsResult getRange(InstanceData &data, SearchHelpers &components, DdsId column, FnT &&f) {
        DdsId table = data.columns.table[column];
        if (auto aosIndex = components.tableAosData[table]) {
            auto &bytes = data.aosTables.data[*aosIndex];
            auto offset = data.columns.aosColumnOffset[column];
            auto stride = data.aosTables.rowSize[*aosIndex];
            dds::DataRange<T> range{bytes, offset, stride};
            return f(range);
        } else {
            auto &bytes = data.columns.soaColumnData[column];
            auto beg = reinterpret_cast<T *>(bytes.begin());
            auto end = beg + bytes.size() / sizeof(T);
            dds::Range<T> range{beg, end};
            return f(range);
        }
    }
}

