#include "HashTable.h"

#include <assert.h>
#include <memory.h>

HashTable::HashTable(uint32_t indexSize) 
{
    hash = nullptr, nextIndex = nullptr;
    Resize(indexSize);
}

HashTable::HashTable(uint32_t hashSize, uint32_t indexSize) 
{
    hash = nullptr, nextIndex = nullptr;
    Resize(hashSize, indexSize);
}

HashTable::~HashTable() 
{
    Free();
}

void HashTable::Resize(uint32_t indexSize) 
{
    Resize(Math::RoundUpToPowerOfTwo(indexSize >> 1), indexSize);
}

void HashTable::Resize(uint32_t hashSize, uint32_t indexSize) 
{
    Free();
    assert((hashSize & (hashSize - 1)) == 0);

    this->hashSize = hashSize;
    hashMask = this->hashSize - 1;
    this->indexSize = indexSize;
    hash = new uint32_t[this->hashSize];
    nextIndex = new uint32_t[this->indexSize];
    memset(hash, 0xff, this->hashSize * 4);
}

void HashTable::ResizeIndex(uint32_t indexSize)
{
    uint32_t* indexs = new uint32_t[indexSize];
    memcpy(indexs, nextIndex, sizeof(uint32_t) * this->indexSize);
    delete[] nextIndex;
    nextIndex = indexs;
    this->indexSize = indexSize;
}

void HashTable::Clear() 
{
    memset(hash, 0xff, hashSize * 4);
}