#pragma once

#include "dds/dds.h"
#include "allocator.hpp"
#include <cista/containers.h>
#include <cista/mmap.h>
#include <cista/targets/buf.h>

namespace std {
    template<>
    struct hash<cista::raw::string> {
        std::size_t operator()(cista::raw::string const &s) const noexcept {
            return std::hash<std::string_view>{}(std::string_view{s.begin(), s.size()});
        }
    };

    template<>
    struct hash<cista::offset::string> {
        std::size_t operator()(cista::offset::string const &s) const noexcept {
            return std::hash<std::string_view>{}(std::string_view{s.begin(), s.size()});
        }
    };
}

namespace dds {
    namespace data = cista::raw;

    struct TableData {
        data::vector<data::string> name{};
        data::vector<DdsSize> length{};
    };

    struct ColumnData {
        data::vector<data::string> name{};
        data::vector<DdsDataType> type{};
        data::vector<DdsId> table{};
        data::vector<DdsSize> aosColumnOffset{};
    };

    struct AosTableData {
        data::vector<DdsId> table{};
        data::vector<DdsSize> rowSize{};
    };

    struct InstanceData {
        TableData tables;
        ColumnData columns;
        AosTableData aosTables;
    };

    struct SerializeTablesData {
        data::vector<data::vector<uint8_t>> columnAosData;
        data::vector<data::vector<uint8_t>> soaTableData;
    };

    struct SerializeData {
        InstanceData instanceData;
        SerializeTablesData tablesData;
    };

    using DataPtr = std::unique_ptr<dds::InstanceData, void (*)(dds::InstanceData *)>;

    struct SerializeInfo {
        std::string path;
        std::optional<cista::buf<cista::mmap>> mmap;
        DataPtr data{nullptr, [](dds::InstanceData *) {}};
    };
    SerializeInfo loadSerializeData(DdsInstanceCreateFlags flags, const char *file);

    struct AllocatorData {
        std::vector<std::vector<uint8_t, DataAllocator<uint8_t>>> aosData;
        std::vector<std::vector<uint8_t, DataAllocator<uint8_t>>> soaData;
    };
    AllocatorData loadAllocatorData(InstanceData &data, DdsAllocator allocator);
}
