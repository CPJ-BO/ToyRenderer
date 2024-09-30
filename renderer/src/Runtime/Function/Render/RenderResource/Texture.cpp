#include "Texture.h"
#include "Core/Log/log.h"
#include "Core/Util/TimeScope.h"
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
    case TEXTURE_TYPE_3D:           slot = BINDLESS_SLOT_TEXTURE_3D;        break;
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
    case TEXTURE_TYPE_3D:           viewType = VIEW_TYPE_3D;        break;
    default:                        LOG_FATAL("Unsupported texture type!"); }    

    return viewType;
}

Texture::Texture(const std::string& path)
: textureType(TEXTURE_TYPE_2D)  // TODO 默认
, format(FORMAT_R8G8B8A8_SRGB)  
, arrayLayer(1)
{
    this->paths.push_back(path);
    LoadFromFile();
}

Texture::Texture(const std::vector<std::string>& paths, TextureType type)
: textureType(type)             // TODO 默认
, format(FORMAT_R8G8B8A8_SRGB)  
, arrayLayer(paths.size())
{
    this->paths = paths;
    LoadFromFile();
}

Texture::Texture(TextureType type, RHIFormat format, Extent3D extent, uint32_t arrayLayer, uint32_t mipLevels)
: textureType(type)
, format(format)
, extent(extent)
, arrayLayer(arrayLayer)
{
    this->mipLevels = mipLevels == 0 ? 
                        (uint32_t)(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1 : 
                        mipLevels;
    InitRHI();
}

Texture::~Texture()
{
    if(!EngineContext::Destroyed() && textureID != 0) EngineContext::RenderResource()->ReleaseBindlessID(textureID, TextureTypeToBindlessSlot(textureType));
}

void Texture::OnLoadAsset()
{   
    if(paths.size() > 0)    LoadFromFile();
    else                    InitRHI();
}

void Texture::InitRHI()
{
    ResourceType resourceType = (textureType == TEXTURE_TYPE_CUBE) ? (RESOURCE_TYPE_TEXTURE_CUBE | RESOURCE_TYPE_TEXTURE) : RESOURCE_TYPE_TEXTURE;
    if(IsRWFormat(format))      resourceType |= RESOURCE_TYPE_RW_TEXTURE;       // TODO由外部设置做选择？是否有什么开销
    if(IsRWFormat(format))      resourceType |= RESOURCE_TYPE_RENDER_TARGET;    // 

    TextureAspectFlags aspects =    IsDepthStencilFormat(format) ? TEXTURE_ASPECT_DEPTH_STENCIL :
                                    IsDepthFormat(format) ? TEXTURE_ASPECT_DEPTH :
                                    IsStencilFormat(format) ? TEXTURE_ASPECT_STENCIL : TEXTURE_ASPECT_COLOR;

    RHITextureInfo textureInfo = {
        .format = format,
        .extent = extent,
        .arrayLayers = arrayLayer,
        .mipLevels = mipLevels,
        .memoryUsage = MEMORY_USAGE_GPU_ONLY,
        .type = resourceType,
        .creationFlag = TEXTURE_CREATION_NONE};
    texture = EngineContext::RHI()->CreateTexture(textureInfo);

    RHITextureViewInfo textureViewInfo = {
        .texture = texture,
        .format = format,
        .viewType = TextureTypeToViewType(textureType),
        .subresource = { aspects, 0, mipLevels, 0, arrayLayer}};
    textureView = EngineContext::RHI()->CreateTextureView(textureViewInfo);

    // EngineContext::RHI()->GetImmediateCommand()->TextureBarrier({texture, RESOURCE_STATE_UNDEFINED, RESOURCE_STATE_SHADER_RESOURCE});
}

void Texture::LoadFromFile()
{
    if(textureType == TEXTURE_TYPE_CUBE && paths.size() != 6)
    {
        LOG_DEBUG("Wrong file num with texture type cube!"); 
        return;        
    }
    if(textureType == TEXTURE_TYPE_3D)
    {
        LOG_DEBUG("3D texture file is not supported for now!"); 
        return;        
    }

    bool initRHI = false;
    for(uint32_t i = 0; i < paths.size(); i++)
    {
        std::vector<uint8_t> data;
        EngineContext::File()->LoadBinary(paths[i], data);

        int width, height, channels;
        stbi_info_from_memory(data.data(), data.size(), &width, &height, &channels);
        stbi_uc* pixels = stbi_load_from_memory(data.data(), data.size(), &width, &height, &channels, 4);   // 这个函数非常慢,10~100ms
        uint32_t bufferSize = width * height * sizeof(uint32_t);

        // bool is16Bit = stbi_is_16_bit_from_memory(data.data(), data.size());
        // bool hdr = stbi_is_hdr_from_memory(data.data(), data.size());
        // uint32_t size = extent.width * extent.height * (uint32_t)path.size() * sizeof(uint32_t);     //RGBA8，4字节每像素

        if(!initRHI)
        {
            initRHI = true;

            extent = {(uint32_t)width, (uint32_t)height, 1};         
            mipLevels = (uint32_t)(std::floor(std::log2(std::max(width, height)))) + 1;

            InitRHI();
        }

        // 拷贝纹理内存
        RHIBufferInfo bufferInfo = {
            .size = bufferSize,
            .memoryUsage = MEMORY_USAGE_CPU_ONLY,
            .type = RESOURCE_TYPE_BUFFER,
            .creationFlag = BUFFER_CREATION_PERSISTENT_MAP
        };
        RHIBufferRef stagingBuffer = EngineContext::RHI()->CreateBuffer(bufferInfo);
        memcpy(stagingBuffer->Map(), pixels, bufferSize);   
        EngineContext::RHI()->GetImmediateCommand()->TextureBarrier(
            {texture, 
            RESOURCE_STATE_UNDEFINED, RESOURCE_STATE_TRANSFER_DST,
            {TEXTURE_ASPECT_COLOR, 0, mipLevels, i, 1}});      
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
    EngineContext::RHI()->GetImmediateCommand()->Flush();

    // 分配bindless，仅当从文件读入时使用
    textureID = EngineContext::RenderResource()->AllocateBindlessID({ 
        .resourceType = RESOURCE_TYPE_TEXTURE, 
        .textureView = textureView}, 
        TextureTypeToBindlessSlot(textureType));
}