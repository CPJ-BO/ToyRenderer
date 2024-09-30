#include "SkyboxComponent.h"
#include "Core/Math/Math.h"
#include "Function/Framework/Component/TransformComponent.h"
#include "Function/Global/Definations.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "Function/Render/RenderResource/Material.h"
#include "Function/Render/RenderResource/Model.h"
#include "Function/Render/RenderResource/Shader.h"
#include "Function/Render/RenderResource/Texture.h"
#include <memory>

CEREAL_REGISTER_TYPE(SkyboxComponent)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, SkyboxComponent)

SkyboxComponent::SkyboxComponent()
{
    objectID = EngineContext::RenderResource()->AllocateObjectID();

    vertexShader = std::make_shared<Shader>(EngineContext::File()->ShaderPath() + "default/skybox.vert.spv", SHADER_FREQUENCY_VERTEX);
    fragmentShader = std::make_shared<Shader>(EngineContext::File()->ShaderPath() + "default/skybox.frag.spv", SHADER_FREQUENCY_FRAGMENT);

    material = std::make_shared<Material>();
    material->SetRenderQueue(10000);
    material->SetRenderPassMask(PASS_MASK_FORWARD_PASS);
    material->SetCullMode(CULL_MODE_NONE);
    material->SetDepthTest(true);
    material->SetDepthWrite(false);
    material->SetUseForDepthPass(false);
    material->SetCastShadow(false);
    material->SetVertexShader(vertexShader);
    material->SetFragmentShader(fragmentShader);

    // ModelProcessSetting processSetting =  {
    //     .smoothNormal = false,
    //     .flipUV = false,
    //     .loadMaterials = false,
    //     .tangentSpace = true,
    //     .generateBVH = false,
    //     .generateCluster = false,
    //     .generateVirtualMesh = false,
    //     .cacheCluster = false
    // };
    // model = std::make_shared<Model>("Asset/BuildIn/Model/Basic/cube.obj", processSetting);
    // EngineContext::Asset()->SaveAsset(model, "Asset/BuildIn/Model/Basic/cube.asset");
    model = EngineContext::Asset()->GetOrLoadAsset<Model>("Asset/BuildIn/Model/Basic/cube.asset");
}

SkyboxComponent::~SkyboxComponent()
{
    if(!EngineContext::Destroyed() && objectID != 0) 
    {
        EngineContext::RenderResource()->ReleaseObjectID(objectID); 
    }
}

void SkyboxComponent::Load()
{
    BeginLoadAssetBind()
    LoadAssetBind(Texture, skyboxTexture)
    EndLoadAssetBind
}

void SkyboxComponent::Save()
{
    BeginSaveAssetBind()
    SaveAssetBind(skyboxTexture)
    EndSaveAssetBind
}

void SkyboxComponent::Init()
{
    Component::Init();

    if(skyboxTexture) material->SetTextureCube(skyboxTexture, 0);
}

void SkyboxComponent::Tick(float deltaTime)
{
    auto cameraComponent = EngineContext::World()->GetActiveScene()->GetActiveCamera();
    if(!cameraComponent) return;


    Vec3 position = cameraComponent->GetPosition();
    Vec3 scale = Vec3(200, 200, 200);
    Quaternion rotation = Quaternion::Identity();

    Mat4 transform = Mat4::Identity();
    transform.block<3,3>(0,0) = rotation.toRotationMatrix() * scale.asDiagonal();
    transform.block<3,1>(0,3) = position;


    ObjectInfo info = {
        .model = transform,
        .materialID = material->GetMaterialID(),
        .vertexID = model->Submesh(0).vertexBuffer->vertexID,
        .indexID = model->Submesh(0).indexBuffer->indexID,
        .sphere = model->Submesh(0).mesh->sphere,
        .box = model->Submesh(0).mesh->box,
        .debugData = Vec4::Zero()
    };

    EngineContext::RenderResource()->SetObjectInfo(info, objectID);
}

void SkyboxComponent::SetSkyboxTexture(TextureRef texture)       
{ 
    if(texture->GetTextureType() != TEXTURE_TYPE_CUBE)  
    {
        ENGINE_LOG_WARN("SkyboxComponent requires skybox texture type of cube!");
        return;
    }

    skyboxTexture = texture; 
    material->SetTextureCube(skyboxTexture, 0);
}

void SkyboxComponent::CollectDrawBatch(std::vector<DrawBatch>& batches)
{
    batches.push_back({
        .objectID = objectID,
        .vertexBuffer = model->Submesh(0).vertexBuffer,
        .indexBuffer = model->Submesh(0).indexBuffer,
        .material = material
    });
}