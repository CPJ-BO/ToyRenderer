#include "Sampler.h"
#include "Function/Global/EngineContext.h"

Sampler::Sampler()
{
    RHISamplerInfo samplerInfo = {
        .minFilter = FILTER_TYPE_LINEAR,
        .magFilter = FILTER_TYPE_LINEAR,
        .mipmapMode = MIPMAP_MODE_LINEAR,
        .addressModeU = ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = ADDRESS_MODE_CLAMP_TO_EDGE,
        .compareFunction = COMPARE_FUNCTION_NEVER,
        .mipLodBias = 0.0f,
        .maxAnisotropy = 0.0f};

    sampler = EngineContext::RHI()->CreateSampler(samplerInfo);
}

Sampler::Sampler(AddressMode addressMode, FilterType filterType, MipMapMode mipmapMode, float maxAnisotropy, SamplerReductionMode reductionMode)
{
    RHISamplerInfo samplerInfo = {
        .minFilter = filterType,
        .magFilter = filterType,
        .mipmapMode = mipmapMode,
        .addressModeU = addressMode,
        .addressModeV = addressMode,
        .addressModeW = addressMode,
        .compareFunction = COMPARE_FUNCTION_NEVER,
        .reductionMode = reductionMode,
        .mipLodBias = 0.0f,
        .maxAnisotropy = maxAnisotropy};

    sampler = EngineContext::RHI()->CreateSampler(samplerInfo);
}