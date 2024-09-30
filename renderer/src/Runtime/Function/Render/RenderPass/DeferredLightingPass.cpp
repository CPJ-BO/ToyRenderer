#include "DeferredLightingPass.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RHI/RHIStructs.h"

void DeferredLightingPass::Init()
{
    auto backend = EngineContext::RHI();

    computeShader = Shader(EngineContext::File()->ShaderPath() + "default/deferred_lighting.comp.spv", SHADER_FREQUENCY_COMPUTE);

    RHIRootSignatureInfo rootSignatureInfo = {};
    rootSignatureInfo.AddEntry(EngineContext::RenderResource()->GetPerFrameRootSignature()->GetInfo())
                     .AddEntry({1, 0, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_TEXTURE})
                     .AddEntry({1, 1, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_TEXTURE})
                     .AddEntry({1, 2, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_TEXTURE})
                     .AddEntry({1, 3, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_TEXTURE})
                     .AddPushConstant({128, SHADER_FREQUENCY_COMPUTE});
    rootSignature = backend->CreateRootSignature(rootSignatureInfo);

    RHIComputePipelineInfo pipelineInfo     = {};
    pipelineInfo.computeShader              = computeShader.shader;
    pipelineInfo.rootSignature              = rootSignature;
    computePipeline   = backend->CreateComputePipeline(pipelineInfo);
}   

void DeferredLightingPass::Build(RDGBuilder& builder) 
{
    Extent2D windowExtent = EngineContext::Render()->GetWindowsExtent();

    RDGTextureHandle diffuse    = builder.GetTexture("G-Buffer Diffuse/Roughness");
    RDGTextureHandle normal     = builder.GetTexture("G-Buffer Normal/Metallic");
    RDGTextureHandle emission   = builder.GetTexture("G-Buffer Emission");

    RDGTextureHandle outColor = builder.CreateTexture("Mesh Pass Out Color") 
        .Exetent({windowExtent.width, windowExtent.height, 1})
        .Format(EngineContext::Render()->GetHdrColorFormat())
        .ArrayLayers(1)
        .MipLevels(1)
        .MemoryUsage(MEMORY_USAGE_GPU_ONLY)
        .AllowReadWrite()
        .AllowRenderTarget()
        .Finish();  

    RDGComputePassHandle pass = builder.CreateComputePass(GetName())
        .RootSignature(rootSignature)
        .ReadWrite(1, 0, 0, diffuse)
        .ReadWrite(1, 1, 0, normal)
        .ReadWrite(1, 2, 0, emission)
        .ReadWrite(1, 3, 0, outColor)
        .Execute([&](RDGPassContext context) {       

            RHICommandListRef command = context.command; 
            command->SetComputePipeline(computePipeline);
            command->BindDescriptorSet(EngineContext::RenderResource()->GetPerFrameDescriptorSet(), 0);
            command->BindDescriptorSet(context.descriptors[1], 1);
            command->PushConstants(&setting, sizeof(DeferredLightingSetting), SHADER_FREQUENCY_COMPUTE);
            command->Dispatch(  EngineContext::Render()->GetWindowsExtent().width / 16, 
                                EngineContext::Render()->GetWindowsExtent().height / 16, 
                                1);
        })
        .Finish();
}
