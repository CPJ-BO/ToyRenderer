#pragma once

#include "Function/Render/RHI/RHIStructs.h"

class Sampler 
{
public:
    Sampler();
    Sampler(AddressMode addressMode = ADDRESS_MODE_CLAMP_TO_EDGE,   
            FilterType filterType = FILTER_TYPE_LINEAR, 
            MipMapMode mipmapMode = MIPMAP_MODE_LINEAR,
            float maxAnisotropy = 0.0f,
            SamplerReductionMode reductionMode = SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE);
    ~Sampler() {};

    RHISamplerRef sampler;
};
typedef std::shared_ptr<Sampler> SamplerRef;
