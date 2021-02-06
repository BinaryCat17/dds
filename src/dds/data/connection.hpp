#pragma once

#include "dds/data/connection.hpp"

namespace dds {
    struct ColumnConnections {
        std::unordered_map<DdsId, dds::Connection> single{};
        std::unordered_map<DdsId, dds::MultiConnection> multi{};
    };

    template<typename C1, typename T1, typename C2>
    DdsResult insertConnection(ColumnConnections &connections, DdsId column, DdsConnectionType type,
            C1 &childConnection, T1 &childParentMember, C2 &parentConnection) {
        auto singleIter = connections.single.find(column);
        if (singleIter != connections.single.end()) {
            return DDS_RESULT_ALREADY_CONNECTED;
        }
        auto multiIter = connections.multi.find(column);
        if (multiIter != connections.multi.end()) {
            return DDS_RESULT_ALREADY_CONNECTED;
        }

        if (type == DDS_CONNECTION_SINGLE) {
            connections.single.emplace(childConnection, childParentMember, parentConnection);
        } else {
            connections.multi.emplace(childConnection, childParentMember, parentConnection);
        }

        return DDS_RESULT_SUCCESS;
    }

    template<typename FnT>
    DdsResult getType(DdsDataType type, FnT &&f) {
        switch (type) {
            case DDS_STRING16_TYPE:
                return f(dds::String16{});
            case DDS_STRING64_TYPE:
                return f(dds::String64{});
            case DDS_STRING256_TYPE:
                return f(dds::String256{});
            case DDS_FLOAT_TYPE:
                return f(float{});
            case DDS_DOUBLE_TYPE:
                return f(double{});
            case DDS_INT32_TYPE:
                return f(int32_t{});
            case DDS_UINT32_TYPE:
                return f(uint32_t{});
            case DDS_INT64_TYPE:
                return f(int64_t{});
            case DDS_UINT64_TYPE:
                return f(uint64_t{});
            case DDS_VEC2F_TYPE:
            case DDS_VEC3F_TYPE:
            case DDS_VEC4F_TYPE:
            case DDS_MAT3F_TYPE:
            case DDS_MAT4F_TYPE:
            default:
                return DDS_RESULT_INVALID_TYPE;
        }
    }

    template<typename FnT>
    DdsResult getTypeRange(InstanceData &data, DdsDataType type, InstanceComponents &components,
            DdsId column, FnT &&f) {
        return dds::getType(type, [&data, &components, &f, column](auto val) {
            using T = std::decay_t<decltype(val)>;
            return dds::getRange<T>(data, components, column, f);
        });
    }
}