#pragma once

#include "../dds.h"
#include <memory>
#include <vector>
#include <span>
#include "DataString.hpp"

namespace dds {
    using Index = DdsIndex;
    using Size = DdsSize;

    struct Vec2F {
        float x;
        float y;
    };

    struct Vec3F {
        float x;
        float y;
        float z;
    };

    struct Vec4F {
        float x;
        float y;
        float z;
        float w;
    };

    using Mat3F = std::array<std::array<float, 3>, 3>;
    using Mat4F = std::array<std::array<float, 4>, 4>;

    enum class Result {
        Undefined,
        Success,
        TableNotExist,
        ColumnNotExist,
        TableAlreadyExist,
        ColumnAlreadyExist,
        TableNotLinear,
        TableLinear,
        InvalidData,
        InvalidType,
    };

    enum class InstanceCreateFlags {
        Mmap = 0x00000001,
    };

    InstanceCreateFlags operator|(InstanceCreateFlags fl, InstanceCreateFlags fr) {
        return static_cast<InstanceCreateFlags>(static_cast<unsigned>(fl) |
                                                static_cast<unsigned>(fr));
    }

    InstanceCreateFlags operator&(InstanceCreateFlags fl, InstanceCreateFlags fr) {
        return static_cast<InstanceCreateFlags>(static_cast<unsigned>(fl) &
                                                static_cast<unsigned>(fr));
    }

    enum class TableType {
        SOA,
        AOS,
        AOS_Pack,
        AOS_Std140,
    };

    enum class SynchronizeFlags {
        Disk = 0x00000001,
    };

    SynchronizeFlags operator|(SynchronizeFlags fl, InstanceCreateFlags fr) {
        return static_cast<SynchronizeFlags>(static_cast<unsigned>(fl) |
                                             static_cast<unsigned>(fr));
    }

    SynchronizeFlags operator&(SynchronizeFlags fl, InstanceCreateFlags fr) {
        return static_cast<SynchronizeFlags>(static_cast<unsigned>(fl) &
                                             static_cast<unsigned>(fr));
    }

    enum class DataType {
        String16,
        String64,
        String256,
        Float,
        Double,
        Int32,
        Uint32,
        Int64,
        Uint64,
        Vec2F,
        Vec3F,
        Vec4F,
        Mat3F,
        Mat4F,
    };

    template<typename T>
    constexpr DataType getDataType() {
        if constexpr (std::is_same_v<T, String16>) {
            return DataType::String16;
        } else if constexpr (std::is_same_v<T, String64>) {
            return DataType::String64;
        } else if constexpr (std::is_same_v<T, String256>) {
            return DataType::String256;
        } else if constexpr (std::is_same_v<T, float>) {
            return DataType::Float;
        } else if constexpr (std::is_same_v<T, double>) {
            return DataType::Double;
        } else if constexpr (std::is_same_v<T, int32_t>) {
            return DataType::Int32;
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            return DataType::Uint32;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return DataType::Int64;
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return DataType::Uint64;
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return DataType::Vec2F;
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return DataType::Vec3F;
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return DataType::Vec4F;
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return DataType::Mat3F;
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return DataType::Mat4F;
        }
    }

    struct ColumnInfo {
        char const *name = nullptr;
        DataType type = DataType::Uint64;
    };

    template<typename T>
    struct ColumnData {
        char const *name = nullptr;
        std::span<T> data{};
    };

    class Instance {
    public:
        template<typename RC>
        explicit Instance(DdsInstanceCreateFlags flags, RC const &f, char const *file) {
            ddsSetThreadData(native, &f);
            native = std::unique_ptr(ddsCreateInstance([](DdsResult r, void *pData) {
                (*static_cast<RC const *>(pData))(static_cast<Result>(r));
            }, static_cast<DdsInstanceCreateFlags>(flags), file), ddsDeleteInstance);
        }

        void synchronize(SynchronizeFlags flags) {
            ddsSynchronize(*native, static_cast<DdsSynchronizeFlags>(flags));
        }

        template<typename RC>
        void createTable(RC const &f, TableCreateFlags flags, char const *name) {
            ddsSetThreadData(*native, &f);
            ddsCreateTable(*native, [](DdsResult r, void *pData) {
                (*static_cast<RC const *>(pData))(static_cast<Result>(r));
            }, static_cast<DdsTableType>(flags), name);
        }

        template<typename RC>
        void deleteTable(RC const &f, char const *name) {
            ddsSetThreadData(*native, &f);
            ddsDeleteTable(*native, [](DdsResult r, void *pData) {
                (*static_cast<RC const *>(pData))(static_cast<Result>(r));
            }, name);
        }

        template<typename RC>
        void createColumn(RC const &f, DataType type, char const *table, char const *column) {
            ddsSetThreadData(*native, &f);
            ddsCreateColumn(*native, [](DdsResult r, void *pData) {
                (*static_cast<RC const *>(pData))(static_cast<Result>(r));
            }, static_cast<DdsDataType>(type), table, column);
        }

        template<typename RC>
        void deleteColumn(RC const &f, char const *table, char const *column) {
            ddsSetThreadData(*native, &f);
            ddsDeleteColumn(*native, [](DdsResult r, void *pData) {
                (*static_cast<RC const *>(pData))(static_cast<Result>(r));
            }, table, column);
        }

        template<typename DT, typename RC>
        void push(RC const &f, char const *table, std::span<ColumnData<DT>> const &data) {
            ddsSetThreadData(*native, &f);

            std::vector<DdsColumn> columnInfos;
            std::vector<DdsData> columnData;

            for (ColumnData<DT> d : data) {
                columnInfos.push_back({d.name, getDataType<DT>()});
                columnData.push_back({d.data.data(), d.data.size_bytes()});
            }

            ddsPush(*native, [](DdsResult r, void *pData) {
                        (*static_cast<RC const *>(pData))(static_cast<Result>(r));
                    }, table, static_cast<uint32_t>(data.size()),
                    columnInfos.data(), columnData.data());
        }

        template<typename RC>
        void pop(RC const &f, char const *table) {
            ddsSetThreadData(*native, &f);
            ddsPush(*native, [](DdsResult r, void *pData) {
                (*static_cast<RC const *>(pData))(static_cast<Result>(r));
            }, table);
        }

        template<typename DT, typename RC>
        void insert(RC const &f, char const *table, Index pos,
                std::span<ColumnData<DT>> const &data) {
            ddsSetThreadData(*native, &f);

            std::vector<DdsColumn> columnInfos;
            std::vector<DdsData> columnData;

            for (ColumnData<DT> d : data) {
                columnInfos.push_back({d.name, getDataType<DT>()});
                columnData.push_back({d.data.data(), d.data.size_bytes()});
            }

            ddsInsert(*native, [](DdsResult r, void *pData) {
                        (*static_cast<RC const *>(pData))(static_cast<Result>(r));
                    }, table, pos, static_cast<uint32_t>(data.size()),
                    columnInfos.data(), columnData.data());

        }

        template<typename RC>
        void remove(RC const &f, char const *table, Index pos) {
            ddsSetThreadData(*native, &f);
            ddsRemove(*native, [](DdsResult r, void *pData) {
                (*static_cast<RC const *>(pData))(static_cast<Result>(r));
            }, table, pos);
        }

        std::pair<Result, std::vector<ColumnInfo>> getTableInfo(char const *table) {
            uint32_t size = 0;
            DdsResult r = ddsGetTableInfo(*native, table, &size, nullptr);
            if (r != DDS_RESULT_SUCCESS) {
                return {static_cast<Result>(r), {}};
            }
            std::vector<ColumnInfo> result(size);
            r = ddsGetTableInfo(*native, table, &size,
                    reinterpret_cast<DdsColumn *>(result.data()));
            return {static_cast<Result>(r), result};
        }

        template<typename DT>
        std::pair<Result, std::span<DT>> getLinear(char const *table) {
            DdsData data;
            DdsResult r = ddsGetLinear(*native, {table, getDataType<DT>()}, &data);
        }

        template<typename DT, typename MC>
        Result modifyLinear(char const *table, MC const &f) {
            ddsSetThreadData(*native, &f);
            ddsModifyLinear(*native, table, [](void *data, Size size, void *userData) {
                (*static_cast<MC const *>(userData))(
                        std::span(reinterpret_cast<DT *>(data), size / sizeof(DT)));
            });
        }

        std::pair<Result, float> getColumn(char const *table, char const *column);

        template<typename MC>
        Result modifyColumn(char const *table, char const *column, MC const &f);

    private:
        std::unique_ptr<DdsInstance> native;
    };
}