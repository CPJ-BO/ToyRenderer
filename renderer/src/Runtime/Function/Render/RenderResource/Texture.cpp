#include "Texture.h"
#include "Core/Log/log.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "Function/Render/RenderResource/RenderResourceManager.h"
#include <cstdint>
#include <cstring>
#include <string>

#include <stb/stb_image.h>
#include <vector>

CEREAL_REGISTER_TYPE(Texture)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Asset, Texture)

BindlessSlot TextureTypeToBindlessSlot(TextureType type)
{
    BindlessSlot slot;
    switch (type) {
    case TEXTURE_TYPE_2D:           slot = BINDLESS_SLOT_TEXTURE_2D;        break;
    case TEXTURE_TYPE_2D_ARRAY:     slot = BINDLESS_SLOT_TEXTURE_2D_ARRAY;  break;
    case TEXTURE_TYPE_CUBE:         slot = BINDLESS_SLOT_TEXTURE_CUBE;      break;
    //case TEXTURE_TYPE_3D:           slot = BINDLESS_SLOT_TEXTURE_3D;        break;
    default:                        LOG_FATAL("Unsupported texture type!"); }

    return slot;
}

TextureViewType TextureTypeToViewType(TextureType type)
{
    TextureViewType viewType;
    switch (type) {
    case TEXTURE_TYPE_2D:           viewType = VIEW_TYPE_2D;        break;
    case TEXTURE_TYPE_2D_ARRAY:     viewType = VIEW_TYPE_2D_ARRAY;  break;
    case TEXTURE_TYPE_CUBE:         viewType = VIEW_TYPE_CUBE;      break;
    //case TEXTURE_TYPE_3D:           viewType = VIEW_TYPE_3D;        break;
    default:                        LOG_FATAL("Unsupported texture type!"); }    

    return viewType;
}

Texture::Texture(const std::string& path)
: textureType(TEXTURE_TYPE_2D)  // TODO 默认
, format(FORMAT_R8G8B8A8_SRGB)  
, arrayLayer(1)
{
    this->path.push_back(path);
    LoadFromFile();
}

Texture::Texture(const std::vector<std::string>& path, TextureType type)
: textureType(type)             // TODO 默认
, format(FORMAT_R8G8B8A8_SRGB)  
, arrayLayer(path.size())
{
    this->path = path;
    LoadFromFile();
}

Texture::Texture(TextureType type, TextureFormat format, Extent2D extent, uint32_t arrayLayer)
: textureType(type)
, format(format)
, extent(extent)
, mipLevels((uint32_t)(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1)
, arrayLayer(arrayLayer)
{
    InitRHI();
}

Texture::~Texture()
{
    if(!EngineContext::Destroyed() && textureID != 0) EngineContext::RenderResource()->ReleaseBindlessID(textureID, TextureTypeToBindlessSlot(textureType));
}

void Texture::OnLoadAsset()
{   
    if(path.size() > 0) LoadFromFile();
    else                InitRHI();
}

void Texture::InitRHI()
{
    RHITextureInfo textureInfo = {
        .format = format,
        .extent = {extent.width, extent.height, 1},
        .arrayLayers = arrayLayer,
        .mipLevels = mipLevels,
        .memoryUsage = MEMORY_USAGE_GPU_ONLY,
        .type = RESOURCE_TYPE_TEXTURE,
        .creationFlag = TEXTURE_CREATION_NONE};
    texture = EngineContext::RHI()->CreateTexture(textureInfo);

    RHITextureViewInfo textureViewInfo = {
        .texture = texture,
        .format = format,
        .viewType = TextureTypeToViewType(textureType),
        .subresource = { TEXTURE_ASPECT_COLOR, 0, mipLevels, 0, arrayLayer}};
    textureView = EngineContext::RHI()->CreateTextureView(textureViewInfo);

    RHISamplerInfo samplerInfo = {
        .minFilter = FILTER_TYPE_LINEAR,
        .magFilter = FILTER_TYPE_LINEAR,
        .mipmapMode = MIPMAP_MODE_LINEAR,
        .addressModeU = ADDRESS_MODE_REPEAT,
        .addressModeV = ADDRESS_MODE_REPEAT,
        .addressModeW = ADDRESS_MODE_REPEAT,
        .compareFunction = COMPARE_FUNCTION_NEVER,
        .mipLodBias = 0.0f,
        .maxAnisotropy = 0.0f};
    sampler = EngineContext::RHI()->CreateSampler(samplerInfo);


    textureID = EngineContext::RenderResource()->AllocateBindlessID(
    { .resourceType = RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, 
                    .textureView = textureView, 
                    .sampler = sampler}, 
                    TextureTypeToBindlessSlot(textureType));

    EngineContext::RHI()->GetImmediateCommand()->TextureBarrier({texture, RESOURCE_STATE_UNDEFINED, RESOURCE_STATE_UNORDERED_ACCESS});
}

void Texture::LoadFromFile()
{
    // if( textureType != TEXTURE_TYPE_2D && 
    //     textureType != TEXTURE_TYPE_2D_ARRAY &&
    //     textureType != TEXTURE_TYPE_CUBE)
    // {
    //     LOG_DEBUG("Unsupported texture type for now!"); //暂时不对3D等做支持了
    //     return;
    // }

    if(textureType == TEXTURE_TYPE_CUBE && path.size() != 6)
    {
        LOG_DEBUG("Wrong file num with texture type cube!"); 
        return;        
    }

    bool initRHI = false;
    RHIBufferRef stagingBuffer;
    for(uint32_t i = 0; i < path.size(); i++)
    {
        std::vector<uint8_t> data;
        EngineContext::File()->LoadBinary(path[0], data);

        int width, height, channels;
        stbi_info_from_memory(data.data(), data.size(), &width, &height, &channels);
        stbi_uc* pixels = stbi_load_from_memory(data.data(), data.size(), &width, &height, &channels, 4);
        uint32_t bufferSize = width * height * sizeof(uint32_t);

        // bool is16Bit = stbi_is_16_bit_from_memory(data.data(), data.size());
        // bool hdr = stbi_is_hdr_from_memory(data.data(), data.size());
        // uint32_t size = extent.width * extent.height * (uint32_t)path.size() * sizeof(uint32_t);     //RGBA8，4字节每像素

        if(!initRHI)
        {
            initRHI = true;

            extent = {(uint32_t)width, (uint32_t)height};         
            mipLevels = (uint32_t)(std::floor(std::log2(std::max(width, height)))) + 1;

            InitRHI();

            RHIBufferInfo bufferInfo = {
                .size = bufferSize,
                .memoryUsage = MEMORY_USAGE_CPU_ONLY,
                .type = RESOURCE_TYPE_BUFFER,
                .creationFlag = BUFFER_CREATION_PERSISTENT_MAP
            };
            stagingBuffer = EngineContext::RHI()->CreateBuffer(bufferInfo);
        }

        // 拷贝纹理内存
        EngineContext::RHI()->GetImmediateCommand()->TextureBarrier(
            {texture, 
            RESOURCE_STATE_UNDEFINED, RESOURCE_STATE_TRANSFER_DST,
            {TEXTURE_ASPECT_COLOR, 0, mipLevels, i, 1}});
        memcpy(stagingBuffer->Map(), pixels, bufferSize);    
        EngineContext::RHI()->GetImmediateCommand()->CopyBufferToTexture(stagingBuffer, 0, texture, {TEXTURE_ASPECT_COLOR, 0, i, 1});
        
        stbi_image_free(pixels);
    }

    // 生成mip，转到SRV状态
    EngineContext::RHI()->GetImmediateCommand()->TextureBarrier({texture, 
        RESOURCE_STATE_TRANSFER_DST, RESOURCE_STATE_TRANSFER_SRC, 
                {TEXTURE_ASPECT_COLOR, 0, mipLevels, 0, arrayLayer}});
    EngineContext::RHI()->GetImmediateCommand()->GenerateMips(texture);
    EngineContext::RHI()->GetImmediateCommand()->TextureBarrier({texture, 
        RESOURCE_STATE_TRANSFER_SRC, RESOURCE_STATE_SHADER_RESOURCE, 
                {TEXTURE_ASPECT_COLOR, 0, mipLevels, 0, arrayLayer}});     
}