#include "FXAAPass.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RHI/RHIStructs.h"
#include <cstdint>

void FXAAPass::Init()
{
    auto backend = EngineContext::RHI();

    computeShader = Shader(EngineContext::File()->ShaderPath() + "post_process/fxaa.comp.spv", SHADER_FREQUENCY_COMPUTE);

    RHIRootSignatureInfo rootSignatureInfo = {};
    rootSignatureInfo.AddEntry({0, 0, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_TEXTURE})
                     .AddEntry({0, 1, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_TEXTURE})
                     .AddEntry(EngineContext::RenderResource()->GetSamplerRootSignature()->GetInfo())
                     .AddPushConstant({128, SHADER_FREQUENCY_COMPUTE});
    rootSignature = backend->CreateRootSignature(rootSignatureInfo);

    RHIComputePipelineInfo pipelineInfo     = {};
    pipelineInfo.computeShader              = computeShader.shader;
    pipelineInfo.rootSignature              = rootSignature;
    computePipeline   = backend->CreateComputePipeline(pipelineInfo);
}   

void FXAAPass::Build(RDGBuilder& builder) 
{
    Extent2D windowExtent = EngineContext::Render()->GetWindowsExtent();

    RDGTextureHandle inColor = builder.GetTexture("Bloom Out Color");
    
    RDGTextureHandle outColor = builder.CreateTexture("FXAA Out Color")
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
        .Read(0, 0, 0, inColor)
        .ReadWrite(0, 1, 0, outColor)
        .Execute([&](RDGPassContext context) {       

            RHICommandListRef command = context.command; 
            command->SetComputePipeline(computePipeline);
            command->BindDescriptorSet(context.descriptors[0], 0);
            command->BindDescriptorSet(EngineContext::RenderResource()->GetSamplerDescriptorSet(), 1);
            command->PushConstants(&enable, sizeof(uint32_t), SHADER_FREQUENCY_COMPUTE);
            command->Dispatch(  EngineContext::Render()->GetWindowsExtent().width / 16, 
                                EngineContext::Render()->GetWindowsExtent().height / 16, 
                                1);
        })
        .Finish();
}
