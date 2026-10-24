#pragma once

#include "Buffer.hpp"

#define __SAO_VARS_AS_TEMPLATE_ARGS(T, N) , T
#define __SAO_VARS_AS_ENTRY_MEMBERS(T, N) T* N;
#define __SAO_VARS_COUNT(T, N) + 1

#define SAO_BUFFER(Vars_, Name_, BindingId_) \
    struct Name_ { \
        constexpr static const char* __name = #Name_; \
        constexpr static u32 __bindingId = BindingId_; \
        using Buffer = SAOBuffer<Name_ Vars_(__SAO_VARS_AS_TEMPLATE_ARGS)>; \
        union { \
            u8* __variables[0 Vars_(__SAO_VARS_COUNT)]; \
            struct { \
                Vars_(__SAO_VARS_AS_ENTRY_MEMBERS) \
            }; \
        }; \
    };

template <typename Entry_, typename ...Vars_>
class SAOBuffer;

#define TOTAL_SPRITE_COUNT 0

#include "../../resources/shaders/headers/shared.h"

/*
    This allows for better organization of struct-of-arrays GPU buffers.
*/
template <typename Entry_, typename ...Vars_>
class SAOBuffer {
public:
    Entry_ at(usize index) {
        assert(mappedData);
        Entry_ entry;
        fillEntries<Vars_...>(entry, 0, index);
        return entry;
    }

    Entry_ operator[](usize index) { return at(index); }

    void bind() {
        buffer->bindAsStorageBuffer(Entry_::__bindingId);
    }

    void flushAll() {
        buffer->flush(0, buffer->getSize());
    }

    void flushAllAndUnmap() {
        flushAll();
        buffer->unmap();
        mappedData = nullptr;
    }

    static UPtr<SAOBuffer<Entry_, Vars_...>> create(usize entryCount, GLenum usage, bool shouldMap = true) {
        auto ret = std::make_unique<SAOBuffer<Entry_, Vars_...>>();
        if (ret->init(entryCount, usage, shouldMap))
            return ret;
        else
            return nullptr;
    }

private:
    template <typename Var_>
    usize addVariable(usize index, usize offset) {
        offset = alignUp(offset, alignof(Var_));
        variableOffsets[index] = offset;
        return offset + sizeof(Var_) * entryCount;
    }

    template <typename Var_, typename ...OtherVars_>
    usize addVariables(usize index, usize offset) {
        offset = addVariable<Var_>(index, offset);
        if constexpr (sizeof...(OtherVars_))
            offset = addVariables<OtherVars_...>(index + 1, offset);
        return offset;
    }

    bool init(usize entryCount, GLenum usage, bool shouldMap) {
        this->entryCount = entryCount;

        usize size = addVariables<Vars_...>(0, 0);

        buffer = Buffer::create(Entry_::__name, size, usage);
        if (!buffer)
            return false;

        if (shouldMap)
            mappedData = (u8*)buffer->mapReadWrite();
        return true;
    }

    template <typename Var_>
    void fillEntry(Entry_& entry, usize varIndex, usize dataIndex) {
        entry.__variables[varIndex] = mappedData + variableOffsets[varIndex] + sizeof(Var_) * dataIndex;
    }

    template <typename Var_, typename ...OtherVars_>
    void fillEntries(Entry_& entry, usize varIndex, usize dataIndex) {
        fillEntry<Var_>(entry, varIndex, dataIndex);
        if constexpr (sizeof...(OtherVars_))
            fillEntries<OtherVars_...>(entry, varIndex + 1, dataIndex);
    }

private:
    UPtr<Buffer> buffer;
    u8* mappedData = nullptr;
    usize entryCount;
    usize variableOffsets[sizeof...(Vars_)];
};