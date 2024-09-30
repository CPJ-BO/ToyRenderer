#pragma once

#include "Math.h"

/*
inline uint32_t LowerNearest2Power(uint32_t x) 
{
    while (x & (x - 1)) x ^= (x & -x);
    return x;
}

inline uint32_t UpperNearest2Power(uint32_t x) 
{
    if (x & (x - 1)) {
        while (x & (x - 1)) x ^= (x & -x);
        return x == 0 ? 1 : (x << 1);
    }
    else {
        return x == 0 ? 1 : (x << 1);
    }
}

inline uint32_t highestOneBit(uint32_t i) 
{
    i |= (i >> 1);
    i |= (i >> 2);
    i |= (i >> 4);
    i |= (i >> 8);
    i |= (i >> 16);
    return i - (i >> 1);
}
*/

class HashTable 
{
private:
    uint32_t hashSize;
    uint32_t hashMask;
    uint32_t indexSize;
    uint32_t* hash;
    uint32_t* nextIndex;
    void ResizeIndex(uint32_t indexSize);
public:
    HashTable(uint32_t indexSize = 0);
    HashTable(uint32_t hashSize, uint32_t indexSize);
    ~HashTable();

    void Resize(uint32_t indexSize);
    void Resize(uint32_t hashSize, uint32_t indexSize);

    void Free() 
    {
        hashSize = 0;
        hashMask = 0;
        indexSize = 0;
        delete[] hash;
        hash = nullptr;
        delete[] nextIndex;
        nextIndex = nullptr;
    }
    void Clear();

    void Add(uint32_t key, uint32_t idx) 
    {
        if (idx >= indexSize) 
        {
            ResizeIndex(std::max(32u, Math::RoundUpToPowerOfTwo(idx + 1)));
        }
        key &= hashMask;
        nextIndex[idx] = hash[key];
        hash[key] = idx;
    }

    void Remove(uint32_t key, uint32_t idx) 
    {
        if (idx >= indexSize) return;
        key &= hashMask;
        if (hash[key] == idx) hash[key] = nextIndex[idx];
        else 
        {
            for (uint32_t i = hash[key]; i != ~0u; i = nextIndex[i]) 
            {
                if (nextIndex[i] == idx) 
                {
                    nextIndex[i] = nextIndex[idx];
                    break;
                }
            }
        }
    }

    struct Container 
    {
        uint32_t idx;
        uint32_t* next;
        struct iter {
            uint32_t idx;
            uint32_t* next;
            void operator++() { idx = next[idx]; }
            bool operator!=(const iter& b)const { return idx != b.idx; }
            uint32_t operator*() { return idx; }
        };
        iter begin() { return iter{ idx,next }; }
        iter end() { return iter{ ~0u,nullptr }; }
    };

    Container operator[](uint32_t key) 
    {
        if (hashSize == 0 || indexSize == 0) return Container{ ~0u,nullptr };
        key &= hashMask;
        return Container{ hash[key],nextIndex };
    }
};