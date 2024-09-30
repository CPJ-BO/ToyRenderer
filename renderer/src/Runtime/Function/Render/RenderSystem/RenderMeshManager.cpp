#include "RenderMeshManager.h"
#include "Function/Framework/Component/MeshRendererComponent.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RenderPass/GPUCullingPass.h"
#include "RenderSystem.h"

void RenderMeshManager::Init()
{

}

void RenderMeshManager::Tick()
{
    PrepareMeshPass();
}

void RenderMeshManager::PrepareMeshPass()
{
    ENGINE_TIME_SCOPE(RenderMeshManager::PrepareMeshPass);

    // 回读上一帧的统计数据
    auto cullingPass = std::dynamic_pointer_cast<GPUCullingPass>(EngineContext::Render()->GetPasses()[GPU_CULLING_PASS]);
    if(cullingPass) cullingPass->CollectStatisticDatas();   

    // 遍历场景，获取绘制信息
    // TODO 场景的CPU端剔除
    std::vector<DrawBatch> batches;
    auto rendererComponents = EngineContext::World()->GetActiveScene()->GetComponents<MeshRendererComponent>();     // 场景物体
    for(auto component : rendererComponents) component->CollectDrawBatch(batches);

    auto skybox = EngineContext::World()->GetActiveScene()->GetSkyBox();    // 天空盒
    if(skybox) skybox->CollectDrawBatch(batches);

    // 交给各个meshpass的processor处理
    MeshPassProcessor::ResetGlobalClusterOffset();
    for(auto& pass : EngineContext::Render()->GetMeshPasses())
    {
        if(!pass) continue;
        for(auto& processor : pass->GetMeshPassProcessors())
        {
            processor->Process(batches);
        }
    }
}