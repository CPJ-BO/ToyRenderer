#include "GPUCullingPass.h"
#include "Core/Util/IndexAlloctor.h"
#include "Function/Global/Definations.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "Function/Render/RenderResource/RenderStructs.h"
#include "MeshPass.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

void GPUCullingPass::Init()
{
    auto backend = EngineContext::RHI();

    cullingShader           = Shader(EngineContext::File()->ShaderPath() + "culling/culling.comp.spv", SHADER_FREQUENCY_COMPUTE);
    clusterCullingShader    = Shader(EngineContext::File()->ShaderPath() + "culling/culling_cluster.comp.spv", SHADER_FREQUENCY_COMPUTE);

    RHIRootSignatureInfo rootSignatureInfo = {};
    rootSignatureInfo.AddEntry(EngineContext::RenderResource()->GetPerFrameRootSignature()->GetInfo())
                     .AddEntry({1, 0, 1024, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_BUFFER})     // IndirectMeshDrawDatas
                     .AddEntry({1, 1, 1024, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_BUFFER})     // IndirectMeshDrawCommands
                     .AddEntry({1, 2, 1024, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_BUFFER})     // IndirectClusterDrawDatas
                     .AddEntry({1, 3, 1024, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_BUFFER})     // IndirectClusterDrawCommands
                     .AddEntry({1, 4, 1024, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_BUFFER})     // IndirectClusterGroupDrawDatas
                     .AddEntry({1, 5, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_BUFFER})
                     .AddPushConstant({128, SHADER_FREQUENCY_COMPUTE});
                                 
    rootSignature = backend->CreateRootSignature(rootSignatureInfo);

    RHIComputePipelineInfo pipelineInfo     = {};
    pipelineInfo.computeShader              = cullingShader.shader;
    pipelineInfo.rootSignature              = rootSignature;
    cullingPipeline                         = backend->CreateComputePipeline(pipelineInfo);

    pipelineInfo.computeShader              = clusterCullingShader.shader;
    clusterCullingPipeline                  = backend->CreateComputePipeline(pipelineInfo);
}   

void GPUCullingPass::Build(RDGBuilder& builder) 
{
    auto pass0Builder = builder.CreateComputePass(GetName());
    auto pass1Builder = builder.CreateComputePass(GetName() + " Cluster");
        
    auto& passes = EngineContext::Render()->GetMeshPasses();

    cullingSetting.processSize = 0;
    for(uint32_t i = 0; i < MESH_PASS_TYPE_MAX_CNT; i++)
    {
        indirectBuffers[i].clear();
        if(!passes[i]) continue;
        for(auto& processor : passes[i]->GetMeshPassProcessors())
        {
            indirectBuffers[i].push_back(processor->GetIndirectBuffers());     // 获取全部的间接绘制缓冲
        }
        
        for(uint32_t j = 0; j < indirectBuffers[i].size(); j++)
        {
            uint32_t index = cullingSetting.processSize++;
            cullingSetting.passType[index] = i;  
            cullingSetting.passOffset[index] = j;    
            std::string indexStr = " [" + std::to_string(index) + "][" + std::to_string(j) + "]";

            RDGBufferHandle meshDrawDatas = builder.CreateBuffer("Mesh Draw Datas" + indexStr)
                .Import(indirectBuffers[i][j]->meshDrawDataBuffer.buffer, RESOURCE_STATE_UNDEFINED)
                .Finish();

            RDGBufferHandle meshDrawCommands = builder.CreateBuffer("Mesh Draw Commands" + indexStr)
                .Import(indirectBuffers[i][j]->meshDrawCommandBuffer.buffer, RESOURCE_STATE_UNDEFINED)
                .Finish();

            RDGBufferHandle clusrterDrawDatas = builder.CreateBuffer("Clusrter Draw Datas" + indexStr)
                .Import(indirectBuffers[i][j]->clusterDrawDataBuffer.buffer, RESOURCE_STATE_UNDEFINED)
                .Finish();

            RDGBufferHandle clusrterDrawCommands = builder.CreateBuffer("Clusrter Draw Commands" + indexStr)
                .Import(indirectBuffers[i][j]->clusterDrawCommandBuffer.buffer, RESOURCE_STATE_UNDEFINED)
                .Finish();

            RDGBufferHandle clusrterGroupDrawDatas = builder.CreateBuffer("Clusrter Group Draw Datas" + indexStr)
                .Import(indirectBuffers[i][j]->clusterGroupDrawDataBuffer.buffer, RESOURCE_STATE_UNDEFINED)
                .Finish();

            pass0Builder
                .ReadWrite(1, 0, index, meshDrawDatas)
                .ReadWrite(1, 1, index, meshDrawCommands)
                .ReadWrite(1, 2, index, clusrterDrawDatas)
                .ReadWrite(1, 3, index, clusrterDrawCommands)
                .ReadWrite(1, 4, index, clusrterGroupDrawDatas);

            pass1Builder
                .ReadWrite(1, 0, index, meshDrawDatas)
                .ReadWrite(1, 1, index, meshDrawCommands)
                .ReadWrite(1, 2, index, clusrterDrawDatas)
                .ReadWrite(1, 3, index, clusrterDrawCommands)
                .ReadWrite(1, 4, index, clusrterGroupDrawDatas)                
                .OutputIndirectDraw(meshDrawCommands)       // 输出，作为后续间接绘制的buffer
                .OutputIndirectDraw(clusrterDrawCommands);
        }
    }
    cullingSettingBuffer.SetData(cullingSetting);
    RDGBufferHandle cullingSettings = builder.CreateBuffer("Culling Setting Buffer")
        .Import(cullingSettingBuffer.buffer, RESOURCE_STATE_UNDEFINED)
        .Finish();

    pass0Builder
        .RootSignature(rootSignature)
        .ReadWrite(1, 5, 0, cullingSettings)
        .Execute([&](RDGPassContext context) {       

            RHICommandListRef command = context.command; 
            command->SetComputePipeline(cullingPipeline);
            command->BindDescriptorSet(EngineContext::RenderResource()->GetPerFrameDescriptorSet(), 0);
            command->BindDescriptorSet(context.descriptors[1], 1);
            command->PushConstants(&lodSetting, sizeof(CullingLodSetting), SHADER_FREQUENCY_COMPUTE);
            command->Dispatch(  std::max(MAX_PER_FRAME_OBJECT_SIZE, MAX_PER_FRAME_CLUSTER_GROUP_SIZE) / 256, 
                                1, 
                                1); 
        })
        .Finish();

    pass1Builder
        .RootSignature(rootSignature)
        .ReadWrite(1, 5, 0, cullingSettings)
        .Execute([&](RDGPassContext context) {       

            RHICommandListRef command = context.command; 
            command->SetComputePipeline(clusterCullingPipeline);
            command->BindDescriptorSet(EngineContext::RenderResource()->GetPerFrameDescriptorSet(), 0);
            command->BindDescriptorSet(context.descriptors[1], 1);
            command->PushConstants(&lodSetting, sizeof(CullingLodSetting), SHADER_FREQUENCY_COMPUTE);
            command->Dispatch(  MAX_PER_FRAME_CLUSTER_SIZE / 256, 
                                1, 
                                1);    
        })
        .Finish();
}

void GPUCullingPass::CollectStatisticDatas()
{
    if(!enableStatistics) return;

    auto& passes = EngineContext::Render()->GetMeshPasses();

    for(uint32_t i = 0; i < MESH_PASS_TYPE_MAX_CNT; i++)
    {
        readBackDatas[i].clear();
        if(!passes[i]) continue;
        for(auto& processor : passes[i]->GetMeshPassProcessors())
        {
            auto indirectBuffer = processor->GetIndirectBuffers();    

            ReadBackData readBackData = {};
            indirectBuffer->meshDrawDataBuffer.GetData(&readBackData.meshSetting, sizeof(IndirectSetting), 0);
            indirectBuffer->clusterDrawDataBuffer.GetData(&readBackData.clusterSetting, sizeof(IndirectSetting), 0);
            indirectBuffer->clusterGroupDrawDataBuffer.GetData(&readBackData.clusterGroupSetting, sizeof(IndirectSetting), 0);
            readBackDatas[i].push_back(readBackData);
        }
    }
}


    
