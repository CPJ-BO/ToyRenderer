#include "Material.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RenderResource/Texture.h"
#include "Resource/Asset/Asset.h"
#include <cstdint>

CEREAL_REGISTER_TYPE(Material)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Asset, Material)

void Material::OnLoadAsset()
{
    BeginLoadAssetBind()
    LoadAssetBind(Texture, textureDiffuse)
    LoadAssetBind(Texture, textureNormal)
    LoadAssetBind(Texture, textureArm)
    LoadAssetBind(Texture, textureSpecular)
    LoadAssetArrayBind(Texture, texture2D)
    LoadAssetArrayBind(Texture, textureCube)
    LoadAssetArrayBind(Texture, texture3D)
    EndLoadAssetBind

    Update();
}

void Material::OnSaveAsset()
{
    BeginSaveAssetBind()
    SaveAssetBind(textureDiffuse)
    SaveAssetBind(textureNormal)
    SaveAssetBind(textureArm)
    SaveAssetBind(textureSpecular)
    SaveAssetArrayBind(texture2D)
    SaveAssetArrayBind(textureCube)
    SaveAssetArrayBind(texture3D)
    EndSaveAssetBind
}

Material::Material()
{
    materialID = EngineContext::RenderResource()->AllocateMaterialID();
}

Material::~Material()
{
    if(!EngineContext::Destroyed() && materialID != 0) EngineContext::RenderResource()->ReleaseMaterialID(materialID);
}

void Material::Update()
{
    materialInfo = {};
    materialInfo.roughness = roughness;
    materialInfo.metallic = metallic;
    materialInfo.alphaClip = alphaClip;

    materialInfo.diffuse = diffuse;
    materialInfo.emission = emission;

    if(textureDiffuse)  materialInfo.textureDiffuse = textureDiffuse->textureID;
    if(textureNormal)   materialInfo.textureNormal = textureNormal->textureID;
    if(textureArm)      materialInfo.textureArm = textureArm->textureID;
    if(textureSpecular) materialInfo.textureSpecular = textureSpecular->textureID;

    materialInfo.ints = ints;
    materialInfo.floats = floats;
    materialInfo.colors = colors;
    for(uint32_t i = 0; i < 8; i++) 
    {
        if(texture2D[i]) materialInfo.texture2D[i] = texture2D[i]->textureID;
    }
    for(uint32_t i = 0; i < 4; i++) 
    {
        if(textureCube[i]) materialInfo.textureCube[i] = textureCube[i]->textureID;
        if(texture3D[i]) materialInfo.texture3D[i] = texture3D[i]->textureID;
    }

    EngineContext::RenderResource()->SetMaterialInfo(materialInfo, materialID);
}

