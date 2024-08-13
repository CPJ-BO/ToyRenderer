#include "IndexAlloctor.h"
#include "Core/Log/log.h"
#include <cstdint>

IndexAlloctor::IndexAlloctor(uint32_t maxIndex) 
: maxIndex(maxIndex)
, nextIndex(1)
{}

uint32_t IndexAlloctor::Allocate()
{
    return AllocateRange(1).begin;
}

IndexRange IndexAlloctor::AllocateRange(uint32_t size)
{
    for(auto iter = unusedIndex.begin(); iter != unusedIndex.end(); iter++)
    {
        if(iter->size > size)
        {
            iter->size -= size;
            return { iter->begin + iter->size, size};
        }
        if(iter->size == size)
        {
            IndexRange range = *iter;
            unusedIndex.erase(iter);
            return range;
        }
    }

    if(nextIndex + size > maxIndex) LOG_FATAL("Index is greater than max index!");
    
    IndexRange range = {nextIndex, size};
    nextIndex += size;
    return range;
}

void IndexAlloctor::Release(uint32_t index)
{
    ReleaseRange({index, 1});
}

void IndexAlloctor::ReleaseRange(IndexRange range)
{
    uint32_t end = range.begin + range.size;

    for(auto iter = unusedIndex.begin(); iter != unusedIndex.end(); iter++)
    {
        if(end < iter->begin) 
        {
            unusedIndex.insert(iter, range);
            return;
        }
        if(end == iter->begin) 
        {
            iter->begin = range.begin;
            return;
        }
    }   

    unusedIndex.push_back(range);
}
