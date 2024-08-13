#pragma once

#include "Core/Serialize/Serializable.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "Resource/Asset/Asset.h"
#include <cstdint>
#include <vector>

enum TextureType{
    TEXTURE_TYPE_2D = 0,
    TEXTURE_TYPE_2D_ARRAY,
    TEXTURE_TYPE_CUBE,
    //TEXTURE_TYPE_3D,

    TEXTURE_TYPE_MAX_ENUM,  //
};

// 暂时只支持2D纹理吧
class Texture : public Asset
{
public:
    Texture(const std::string& path);
    Texture(const std::vector<std::string>& path, TextureType type);
    Texture(TextureType type, TextureFormat format, Extent2D extent, uint32_t arrayLayer);
    ~Texture();

    virtual AssetType GetType() override                        { return ASSET_TYPE_TEXTURE; }

    RHITextureRef texture;
    RHITextureViewRef textureView;
    RHISamplerRef sampler;

    void SetData(void* data, uint32_t size);        // TODO
    void SetSampler(RHISamplerInfo samplerInfo);    // TODO

    uint32_t textureID = 0;  

    // inline std::vector<std::string> GetPath()                   { return path; }

    // void SetPath(const std::vector<std::string>& path)          { this->path = path; }
    // void SetPath(const std::string& path)                       { this->path.clear(); this->path.push_back(path); }

protected:
    virtual void OnLoadAsset() override;

    std::vector<std::string> path;
    TextureType textureType;
    TextureFormat format;
    Extent2D extent;
    uint32_t mipLevels;
    uint32_t arrayLayer;

    void InitRHI();
    void LoadFromFile();

private:
    Texture() = default;

    BeginSerailize()
    SerailizeBaseClass(Asset)
    SerailizeEntry(path)
    SerailizeEntry(textureType)
    SerailizeEntry(format)
    SerailizeEntry(extent)
    SerailizeEntry(mipLevels)
    SerailizeEntry(arrayLayer)
    EndSerailize
};
typedef std::shared_ptr<Texture> TextureRef;