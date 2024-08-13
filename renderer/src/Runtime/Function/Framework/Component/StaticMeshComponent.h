#pragma once

#include "Core/Serialize/Serializable.h"
#include "Function/Render/RenderResource/Material.h"
#include "Function/Render/RenderResource/Model.h"
#include "MeshRendererComponent.h"
#include "Resource/Asset/Asset.h"
#include <vector>

class StaticMeshComponent : public MeshRendererComponent, public AssetBinder
{
public:
	virtual void Init() override;
	virtual void Tick(float deltaTime) override;
	virtual void Save()override;

    ModelRef model;
    std::vector<MaterialRef> materials;

private:
    BeginSerailize()
    SerailizeBaseClass(MeshRendererComponent)
    SerailizeBaseClass(AssetBinder)
    EndSerailize
};