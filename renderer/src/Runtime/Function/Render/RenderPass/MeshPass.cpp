#include "MeshPass.h"
#include "Core/Log/Log.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "Function/Render/RenderResource/PipelineCache.h"
#include "Function/Render/RenderResource/RenderStructs.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <set>
#include <unordered_map>
#include <vector>

uint32_t MeshPassProcessor::globalClusterOffset = 0;

std::shared_ptr<MeshPassIndirectBuffers> MeshPassProcessor::GetIndirectBuffers()                                   
{ 
    return indirectBuffers[EngineContext::CurrentFrameIndex()]; 
}

void MeshPassProcessor::Init()
{ 
    for(auto& indirectBuffer : indirectBuffers)
    {
        if(indirectBuffer == nullptr) 
        indirectBuffer = std::make_shared<MeshPassIndirectBuffers>(); 
    }
}

void MeshPassProcessor::Process(const std::vector<DrawBatch>& drawBatches)
{
    ENGINE_TIME_SCOPE(MeshPassProcessor::Process);

    batches.clear();
    drawGeometries.clear();
    drawCommands.clear();
    meshDrawCommand.clear();
    meshDrawInfo.clear();
    clusterDrawCommand.clear();
    clusterDrawInfo.clear();
    clusterGroupDrawInfo.clear();

    for(auto& batch : drawBatches)
    {
        OnCollectBatch(batch);
    }

    for(auto& batch : batches)
    {
        OnBuildDrawInfo(batch); 
    }

    uint32_t pipelineIndex = 0;
    for(auto& pair : drawGeometries)
    {
        assert(pipelineIndex < MAX_PER_PASS_PIPELINE_STATE_COUNT);      // 限制最大的可能管线状态数目
        
        RHIGraphicsPipelineRef pipeline = OnCreatePipeline(pair.first);
        if(pipeline)
        {
            OnBuildDrawCommands(pipelineIndex, pipeline, pair.second);
            pipelineIndex++;
        }
    }

    // 填写cluster的间接绘制命令
    // 由于所有绘制指令里的索引最后指向同一个全局缓冲，需要计算全局的偏移
    uint32_t globalClusterOffset = MeshPassProcessor::GetGlobalClusterOffset();
    uint32_t localClusterOffset = 0;
    for(uint32_t i = 0; i < pipelineIndex; i++)
    {
        clusterDrawCommand.push_back({
            .vertexCount = CLUSTER_TRIANGLE_SIZE * 3,
            .instanceCount = 0,
            .firstVertex = 0,
            .firstInstance = globalClusterOffset + localClusterOffset
        });
        localClusterOffset += clusterCount[i];
    }
    MeshPassProcessor::AddGlobalClusterOffset(localClusterOffset);

    // 将准备好的全部数据提交给GPU端
    IndirectSetting meshDrawSetting = {
        .processSize = (uint32_t)meshDrawInfo.size(),
        .pipelineStateSize = pipelineIndex,
        .drawSize = 0,
        .frustumCull = 0,
        .occlusionCull = 0
    };
    auto buffers = GetIndirectBuffers();
    buffers->meshDrawDataBuffer.SetData(&meshDrawSetting, sizeof(IndirectSetting), 0);
    buffers->meshDrawDataBuffer.SetData(meshDrawInfo.data(), meshDrawInfo.size() * sizeof(IndirectMeshDrawInfo), sizeof(IndirectSetting));
    buffers->meshDrawCommandBuffer.SetData(meshDrawCommand.data(), meshDrawCommand.size() * sizeof(RHIIndirectCommand), 0);

    IndirectSetting clusterDrawSetting = {
        .processSize = (uint32_t)clusterDrawInfo.size(),
        .pipelineStateSize = pipelineIndex,
        .drawSize = 0,
        .frustumCull = 0,
        .occlusionCull = 0
    };
    buffers->clusterDrawDataBuffer.SetData(&clusterDrawSetting, sizeof(IndirectSetting), 0);
    buffers->clusterDrawDataBuffer.SetData(clusterDrawInfo.data(), clusterDrawInfo.size() * sizeof(IndirectClusterDrawInfo), sizeof(IndirectSetting));
    buffers->clusterDrawCommandBuffer.SetData(clusterDrawCommand.data(), clusterDrawCommand.size() * sizeof(RHIIndirectCommand), 0);

    IndirectSetting clusterGroupDrawSetting = {
        .processSize = (uint32_t)clusterGroupDrawInfo.size(),
        .pipelineStateSize = pipelineIndex,
        .drawSize = 0,
        .frustumCull = 0,
        .occlusionCull = 0
    };
    buffers->clusterGroupDrawDataBuffer.SetData(&clusterGroupDrawSetting, sizeof(IndirectSetting), 0);
    buffers->clusterGroupDrawDataBuffer.SetData(clusterGroupDrawInfo.data(), clusterGroupDrawInfo.size() * sizeof(IndirectClusterGroupDrawInfo), sizeof(IndirectSetting));

    // ENGINE_LOG_INFO("Mesh pass processor process size: {} {} {}", meshDrawInfo.size(), clusterDrawInfo.size(), clusterGroupDrawInfo.size());
}

void MeshPassProcessor::Draw(RHICommandListRef command)
{
    for(auto& drawCommand : drawCommands)
    {
        command->SetGraphicsPipeline(drawCommand.pipeline);

        if(drawCommand.meshCommandRange.size > 0)
        {
            command->DrawIndirect(
                drawCommand.indirectMeshCommandBuffer, 
                drawCommand.meshCommandOffset + drawCommand.meshCommandRange.begin * sizeof(RHIIndirectCommand), 
                drawCommand.meshCommandRange.size);
        }  

        if(drawCommand.clusterCommandRange.size > 0)   
        {
            command->DrawIndirect(
            drawCommand.indirectClusterCommandBuffer, 
            drawCommand.clusterCommandOffset + drawCommand.clusterCommandRange.begin * sizeof(RHIIndirectCommand), 
            drawCommand.clusterCommandRange.size);
        }  
    }
}

void MeshPassProcessor::OnCollectBatch(const DrawBatch& batch)
{
    // AddBatch(batch);
}

void MeshPassProcessor::OnBuildDrawInfo(const DrawBatch& batch)
{
    DrawPipelineState pipelineState = {};
    pipelineState.renderQueue       = batch.material->RenderQueue();
    pipelineState.cullMode          = batch.material->CullMode();
    pipelineState.fillMode          = batch.material->GetFillMode();
    pipelineState.depthTest         = batch.material->DepthTest();
    pipelineState.depthWrite        = batch.material->DepthWrite();
    pipelineState.vertexShader      = batch.material->GetVertexShader() ? batch.material->GetVertexShader()->shader : nullptr;
    pipelineState.geometryShader    = batch.material->GetGeometryShader() ? batch.material->GetGeometryShader()->shader : nullptr;
    pipelineState.fragmentShader    = batch.material->GetFragmentShader() ? batch.material->GetFragmentShader()->shader : nullptr;
    pipelineState.clusterRender     = (batch.clusterGroupID.begin > 0 || batch.clusterID.begin > 0) ? true : false;
    pipelineState.meshRender        = !pipelineState.clusterRender;

    DrawGeometryInfo info = {};
    info.objectID                   = batch.objectID;
    info.vertexID                   = batch.vertexBuffer->vertexID;
    info.indexID                    = batch.indexBuffer->indexID;
    info.indexCount                 = batch.indexBuffer->IndexNum();
    info.clusterID                  = batch.clusterID;
    info.clusterGroupID             = batch.clusterGroupID;

    AddDrawInfo(pipelineState, info);
}

RHIGraphicsPipelineRef MeshPassProcessor::OnCreatePipeline(const DrawPipelineState& pipelineState)
{
    RHIGraphicsPipelineInfo pipelineInfo    = {};
    pipelineInfo.vertexShader               = pipelineState.vertexShader;
    pipelineInfo.geometryShader             = pipelineState.geometryShader;
    pipelineInfo.fragmentShader             = pipelineState.fragmentShader;
    pipelineInfo.primitiveType              = PRIMITIVE_TYPE_TRIANGLE_LIST;
    pipelineInfo.rasterizerState            = { pipelineState.fillMode, pipelineState.cullMode, DEPTH_CLIP, 0.0f, 0.0f };
    pipelineInfo.depthStencilState          = { pipelineState.depthCompare, pipelineState.depthTest, pipelineState.depthWrite };

    // 需要各个mesh pass 重载，提供根签名和帧缓冲信息
    // pipelineInfo.rootSignature                   = rootSignature;
    // for(uint32_t i = 0; i < 4; i++) pipelineInfo.blendState.renderTargets[i].enable = false;
    // pipelineInfo.colorAttachmentFormats[0]      = FORMAT_R8G8B8A8_UNORM;
    // pipelineInfo.colorAttachmentFormats[1]      = FORMAT_R8G8B8A8_UNORM;
    // pipelineInfo.colorAttachmentFormats[2]      = FORMAT_R8G8B8A8_UNORM;
    // pipelineInfo.colorAttachmentFormats[3]      = FORMAT_R16G16B16A16_SFLOAT;                                               
    // pipelineInfo.depthStencilAttachmentFormat   = EngineContext::Render()->GetDepthFormat();

    return GraphicsPipelineCache::Get()->Allocate(pipelineInfo).pipeline;   // 构建可能失败返回空，则后续处理将放弃该pipelineState的绘制
}

void MeshPassProcessor::OnBuildDrawCommands(
    uint32_t pipelineIndex, 
    RHIGraphicsPipelineRef pipeline, 
    const std::vector<DrawGeometryInfo>& geometries)
{
    auto buffers = GetIndirectBuffers();

    DrawCommand drawCommand = {
        .pipeline = pipeline,

        .meshCommandRange = { (uint32_t)meshDrawCommand.size(), 0 },    // 下面填size
        .meshCommandOffset = 0,
        .indirectMeshCommandBuffer = buffers->meshDrawCommandBuffer.buffer,

        .clusterCommandRange = { pipelineIndex, 1 },                    // 一个管线状态的cluster共用一个command
        .clusterCommandOffset = 0,
        .indirectClusterCommandBuffer = buffers->clusterDrawCommandBuffer.buffer
    };

    clusterCount[pipelineIndex] = 0; 
    uint32_t meshCount = 0;
    for(auto& geometry : geometries)
    {
        // 虚拟几何体 //////////////////////////////////////////////////////////
        if(geometry.clusterGroupID.begin != 0)       
        {
            clusterCount[pipelineIndex] += geometry.clusterID.size;     // 仍然需要更新该管线状态下最大可能绘制的cluster数目
            // for(uint32_t i = 0; i < geometry.clusterID.size; i++)    // 但是不需要把cluster信息直接提交给缓冲,
            // {                                                        // 而是在GPU端处理group时负责添加
            //     clusterDrawInfo.push_back({
            //         .objectID = geometry.objectID,
            //         .clusterID = geometry.clusterID.begin + i,
            //         .commandID = pipelineIndex                     
            //     });
            // }

            for(uint32_t i = 0; i < geometry.clusterGroupID.size; i++)
            {
                clusterGroupDrawInfo.push_back({
                    .objectID = geometry.objectID,
                    .clusterGroupID = geometry.clusterGroupID.begin + i,
                    .commandID = pipelineIndex
                });
            }
        }
        // 分簇 //////////////////////////////////////////////////////////
        else if(geometry.clusterID.begin != 0)       
        {
            clusterCount[pipelineIndex] += geometry.clusterID.size;
            for(uint32_t i = 0; i < geometry.clusterID.size; i++)
            {
                clusterDrawInfo.push_back({
                    .objectID = geometry.objectID,
                    .clusterID = geometry.clusterID.begin + i,
                    .commandID = pipelineIndex                     
                });
            }
        }
        // 常规渲染 //////////////////////////////////////////////////////////
        else                                        
        {
            meshDrawInfo.push_back({
                .objectID = geometry.objectID, 
                .commandID = (uint32_t)meshDrawCommand.size()
            });
            meshDrawCommand.push_back({
                .vertexCount = geometry.indexCount,
                .instanceCount = 1,                     // TODO 使用同一个顶点和索引缓冲的还能进一步合并？
                .firstVertex = 0,                       // 间接绘制里这样的多个indirect command 有多大的开销？
                .firstInstance = geometry.objectID
            });
            meshCount++;
        }
    }
    drawCommand.meshCommandRange.size = meshCount;  //每个mesh一个command

    AddDrawCommand(drawCommand);
}

