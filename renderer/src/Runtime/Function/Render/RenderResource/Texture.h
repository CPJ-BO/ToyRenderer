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
    TEXTURE_TYPE_3D,

    TEXTURE_TYPE_MAX_ENUM,  //
};

class Texture : public Asset
{
public:
    Texture(const std::string& path);
    Texture(const std::vector<std::string>& paths, TextureType type);
    Texture(TextureType type, RHIFormat format, Extent3D extent, uint32_t arrayLayer = 1, uint32_t mipLevels = 0);
    ~Texture();

    virtual std::string GetAssetTypeName() override 			{ return "Texture Asset"; }
    virtual AssetType GetAssetType() override                   { return ASSET_TYPE_TEXTURE; }

    virtual void OnLoadAsset() override;

    TextureType GetTextureType()                                { return textureType; }

    RHITextureRef texture;
    RHITextureViewRef textureView;

    void SetData(void* data, uint32_t size);        // TODO

    uint32_t textureID = 0;  

    // inline std::vector<std::string> GetPath()                   { return path; }

    // void SetPath(const std::vector<std::string>& path)          { this->path = path; }
    // void SetPath(const std::string& path)                       { this->path.clear(); this->path.push_back(path); }

protected:
    std::vector<std::string> paths;
    TextureType textureType;
    RHIFormat format;
    Extent3D extent;
    uint32_t mipLevels;
    uint32_t arrayLayer;

    void InitRHI();
    void LoadFromFile();

private:
    Texture() = default;

    BeginSerailize()
    SerailizeBaseClass(Asset)
    SerailizeEntry(paths)
    SerailizeEntry(textureType)
    SerailizeEntry(format)
    SerailizeEntry(extent)
    SerailizeEntry(mipLevels)
    SerailizeEntry(arrayLayer)
    EndSerailize

    EnableAssetEditourUI()
};
typedef std::shared_ptr<Texture> TextureRef;