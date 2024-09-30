#pragma once

#include "Core/Math/Math.h"
#include "Core/Serialize/Serializable.h"
#include "Component.h"
#include "Function/Render/RenderPass/MeshPass.h"
#include "Function/Render/RenderResource/Drawable.h"
#include "Function/Render/RenderResource/Material.h"
#include "Function/Render/RenderResource/Model.h"
#include "Function/Render/RenderResource/RenderStructs.h"
#include <cstdint>
#include <vector>

enum MeshRendererMode
{
	RENDER_MODE_DEFAULT = 0,
	RENDER_MODE_CLUSTER,
	RENDER_MODE_VIRTUAL_MESH,

	RENDER_MODE_MAX,//
};

class MeshRendererComponent : public Component, public AssetBinder, public Drawable
{
public:
	MeshRendererComponent() = default;
	~MeshRendererComponent();

	virtual void Load() override;
	virtual void Save() override;
	virtual void Init() override;
	virtual void Tick(float deltaTime) override;

	virtual std::string GetTypeName() override		{ return "Mesh Renderer Component"; }
	virtual ComponentType GetType() override	    { return MESH_RENDERER_COMPONENT; }

	void SetModel(ModelRef model); 					
	ModelRef GetModel()								{ return model; }

	void SetMaterial(MaterialRef material, uint32_t index = 0);
	void SetMaterials(std::vector<MaterialRef> materials, uint32_t firstIndex = 0);
	MaterialRef GetMaterial(uint32_t index);			

	virtual void CollectDrawBatch(std::vector<DrawBatch>& batches) override;

private:
	ModelRef model;
    std::vector<MaterialRef> materials;
	std::vector<ObjectInfo> objectInfos;
	std::vector<uint32_t> objectIDs;

	Mat4 prevModel = Mat4::Identity();

	bool castShadow;					//是否产生阴影（加入shadow map render pass）
	MeshRendererMode renderMode;		//渲染模式

	int32_t materilalInspectMode = 1;		//材质检视模式
	int32_t materilalInspectIndex = 0;		//材质检视下标

private:
    BeginSerailize()
    SerailizeBaseClass(Component)
    SerailizeBaseClass(AssetBinder)
	SerailizeEntry(castShadow)
	SerailizeEntry(renderMode)
	//SerailizeAssetEntry(model)
	//SerailizeAssetArrayEntry(materials)
    EndSerailize
	
	EnableComponentEditourUI()
};

