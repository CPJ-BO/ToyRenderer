#include "ClusterLightingPass.h"
#include "Function/Global/Definations.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RDG/RDGHandle.h"
#include "Function/Render/RHI/RHIStructs.h"

void ClusterLightingPass::Init()
{
    auto backend = EngineContext::RHI();

    computeShader = Shader(EngineContext::File()->ShaderPath() + "culling/cluster_lighting.comp.spv", SHADER_FREQUENCY_COMPUTE);

    RHIRootSignatureInfo rootSignatureInfo = {};
    rootSignatureInfo.AddEntry(EngineContext::RenderResource()->GetPerFrameRootSignature()->GetInfo()); // 不需要单独的描述符，已经在全局里了
                    //  .AddEntry({1, 0, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_TEXTURE})   // LightClusterGrid
                    //  .AddEntry({1, 1, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_BUFFER});   // LightClusterIndex
    rootSignature = backend->CreateRootSignature(rootSignatureInfo);

    RHIComputePipelineInfo pipelineInfo     = {};
    pipelineInfo.computeShader              = computeShader.shader;
    pipelineInfo.rootSignature              = rootSignature;
    computePipeline   = backend->CreateComputePipeline(pipelineInfo);
}   

void ClusterLightingPass::Build(RDGBuilder& builder) 
{
    RDGTextureHandle lightClusterGrid = builder.CreateTexture("Light Cluster Grid")
        .Import(EngineContext::RenderResource()->GetLightClusterGridTexture(), RESOURCE_STATE_UNDEFINED)
        .Finish();

    RDGBufferHandle lightClusterIndex = builder.CreateBuffer("Light Cluster Index")
        .Import(EngineContext::RenderResource()->GetLightClusterIndexBuffer(), RESOURCE_STATE_UNDEFINED)
        .Finish();

    RDGComputePassHandle pass = builder.CreateComputePass(GetName())
        //.RootSignature(rootSignature)
        .ReadWrite(0, 0, 0, lightClusterGrid)   // 不提供根签名时，此处仅用于设置屏障
        .ReadWrite(0, 0, 0, lightClusterIndex)
        .Execute([&](RDGPassContext context) {       

            RHICommandListRef command = context.command; 
            command->SetComputePipeline(computePipeline);
            command->BindDescriptorSet(EngineContext::RenderResource()->GetPerFrameDescriptorSet(), 0);
            command->Dispatch(  1, 
                                1, 
                                LIGHT_CLUSTER_DEPTH);
        })
        .OutputReadWrite(lightClusterGrid)
        .OutputReadWrite(lightClusterIndex)
        .Finish();
}
