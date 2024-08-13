#pragma once

#include "Core/Serialize/Serializable.h"
#include <cstdint>
#include <list>

typedef struct IndexRange
{
    uint32_t begin = 0;
    uint32_t size = 1;

private:
    BeginSerailize()
    SerailizeEntry(begin)
    SerailizeEntry(size)
    EndSerailize
} IndexRange;

// 工具类，用于分配映射关系，0为无效值
// 写复杂了就是一个内存池
class IndexAlloctor   
{
public:
    IndexAlloctor(uint32_t maxIndex = UINT32_MAX);

    uint32_t Allocate();
    IndexRange AllocateRange(uint32_t size);
    void Release(uint32_t index);
    void ReleaseRange(IndexRange range);

    inline uint32_t GetSize() { return maxIndex; }

private:
    uint32_t maxIndex;
    uint32_t nextIndex;
     
    std::list<IndexRange> unusedIndex;

private:
    BeginSerailize()
    SerailizeEntry(maxIndex)
    SerailizeEntry(nextIndex)
    SerailizeEntry(unusedIndex)
    EndSerailize
};