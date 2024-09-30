#pragma once

#include "Math.h"

#include <cstdint>

inline uint32_t MurmurAdd(uint32_t hash, uint32_t elememt)
{
    elememt *= 0xcc9e2d51;
    elememt = (elememt << 15) | (elememt >> (32 - 15));
    elememt *= 0x1b873593;

    hash ^= elememt;
    hash = (hash << 13) | (hash >> (32 - 13));
    hash = hash * 5 + 0xe6546b64;
    return hash;
}

inline uint32_t MurmurMix(uint32_t hash)
{
    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;
    return hash;
}

inline uint32_t Hash(const Vec3& v)
{
    union { float f; uint32_t u; } x, y, z;
    x.f = (v.x() == 0.f ? 0 : v.x());
    y.f = (v.y() == 0.f ? 0 : v.y());
    z.f = (v.z() == 0.f ? 0 : v.z());
    return MurmurMix(MurmurAdd(MurmurAdd(x.u, y.u), z.u));
}

inline uint32_t Hash(const uint32_t h0, uint32_t h1)
{
    return MurmurMix(MurmurAdd(h0, h1));
}

inline uint32_t Hash(const Vec3& first, const Vec3& second)
{
    return Hash(Hash(first), Hash(second));
}

