#pragma once

#include "Component.h"
#include "Function/Render/RenderPass/MeshPass.h"
#include "Function/Render/RenderResource/Drawable.h"
#include "Function/Render/RenderResource/Material.h"
#include "Function/Render/RenderResource/Model.h"
#include "Function/Render/RenderResource/Shader.h"
#include "Function/Render/RenderResource/Texture.h"
#include "Resource/Asset/Asset.h"
#include <cstdint>

class SkyboxComponent : public Component, public AssetBinder, public Drawable
{
public:
	SkyboxComponent();
	~SkyboxComponent();

	virtual void Load() override;
	virtual void Save() override;
	virtual void Init() override;
	virtual void Tick(float deltaTime) override;

    virtual std::string GetTypeName() override		{ return "Skybox Component"; }
	virtual ComponentType GetType() override	    { return SKYBOX_COMPONENT; }

    void SetSkyboxTexture(TextureRef texture);     
    uint32_t GetMaterialID()                        { return material ? material->GetMaterialID() : 0; }  

	virtual void CollectDrawBatch(std::vector<DrawBatch>& batches) override;

private:
    ModelRef model;
    MaterialRef material;

    TextureRef skyboxTexture;

    ShaderRef vertexShader;
    ShaderRef fragmentShader;

    uint32_t objectID = 0;

private:
    BeginSerailize()
    SerailizeBaseClass(Component)
    SerailizeBaseClass(AssetBinder)
    EndSerailize

    EnableComponentEditourUI()
};