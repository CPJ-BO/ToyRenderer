#pragma once

#include "Core/Math/Math.h"
#include "Core/Serialize/Serializable.h"
#include "Function/Render/RenderResource/RenderStructs.h"
#include "Function/Render/RenderResource/Texture.h"
#include "Resource/Asset/Asset.h"
#include <array>
#include <cstdint>
#include <memory>

// 约等于shaderlab文件的抽象，拆成了Material+索引各个shader文件
// 到底应该如何抽象材质类？不是不能参考，但是太复杂了
// 目前选择不提供完整的光栅管线状态设置，仅设置材质的参数信息和着色器信息，（以及对应的延迟/前向管线？）
// TODO 给材质提供Uniform buffer？
class Material : public Asset, public AssetBinder
{
public:
    Material();
    ~Material();

    virtual AssetType GetType() override { return ASSET_TYPE_MATERIAL; }

    Vec4 diffuse;
    Vec4 emission;

    float roughness;
    float metallic;
    float alphaClip;

    std::array<int32_t, 8> ints;
    std::array<float, 8> floats;
    std::array<Vec4, 8> colors;

    TextureRef textureDiffuse;
    TextureRef textureNormal;
    TextureRef textureArm;
    TextureRef textureSpecular;

    std::array<TextureRef, 8> texture2D;
    std::array<TextureRef, 4> textureCube;
    std::array<TextureRef, 4> texture3D;

    MaterialInfo materialInfo;
    uint32_t materialID;

    void Update();

protected:
    virtual void OnLoadAsset() override;
    virtual void OnSaveAsset() override;
    
private:
    BeginSerailize()
    SerailizeBaseClass(Asset)
    SerailizeBaseClass(AssetBinder)
    SerailizeEntry(diffuse)
    SerailizeEntry(emission)
    SerailizeEntry(roughness)
    SerailizeEntry(metallic)
    SerailizeEntry(alphaClip)
    SerailizeEntry(ints)
    SerailizeEntry(floats)
    SerailizeEntry(colors)
    EndSerailize
};
typedef std::shared_ptr<Material> MaterialRef;



