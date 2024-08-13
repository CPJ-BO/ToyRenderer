#include "StaticMeshComponent.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RenderResource/Material.h"
#include "Function/Render/RenderResource/Model.h"
#include "Resource/Asset/Asset.h"



CEREAL_REGISTER_TYPE(StaticMeshComponent)
CEREAL_REGISTER_POLYMORPHIC_RELATION(MeshRendererComponent, StaticMeshComponent)

void StaticMeshComponent::Init()
{
    BeginLoadAssetBind()
    ResizeAssetArray(materials)
    LoadAssetBind(Model, model)
    LoadAssetArrayBind(Material, materials)
    EndLoadAssetBind
}

void StaticMeshComponent::Save()
{
    BeginSaveAssetBind()
    SaveAssetBind(model)
    SaveAssetArrayBind(materials)
    EndSaveAssetBind
}

void StaticMeshComponent::Tick(float deltaTime)
{
    
}

