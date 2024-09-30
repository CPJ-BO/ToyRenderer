#include "PostProcessingPass.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RHI/RHIStructs.h"
#include <cstdint>

void PostProcessingPass::Init()
{
    auto backend = EngineContext::RHI();

    computeShader = Shader(EngineContext::File()->ShaderPath() + "post_process/post_process.comp.spv", SHADER_FREQUENCY_COMPUTE);

    RHIRootSignatureInfo rootSignatureInfo = {};
    rootSignatureInfo.AddEntry({0, 0, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_TEXTURE})
                     .AddEntry({0, 1, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_TEXTURE})
                     .AddEntry({0, 2, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_BUFFER})
                     .AddPushConstant({128, SHADER_FREQUENCY_COMPUTE});
    rootSignature = backend->CreateRootSignature(rootSignatureInfo);

    RHIComputePipelineInfo pipelineInfo     = {};
    pipelineInfo.computeShader              = computeShader.shader;
    pipelineInfo.rootSignature              = rootSignature;
    computePipeline   = backend->CreateComputePipeline(pipelineInfo);
}   

void PostProcessingPass::Build(RDGBuilder& builder) 
{
    Extent2D windowExtent = EngineContext::Render()->GetWindowsExtent();

    RDGTextureHandle fxaaOutColor = builder.GetTexture("FXAA Out Color");

    RDGBufferHandle exposureData = builder.GetBuffer("Exposure Data");

    RDGTextureHandle outColor = builder.CreateTexture("Final Color")
        .Exetent({windowExtent.width, windowExtent.height, 1})
        .Format(EngineContext::Render()->GetColorFormat())
        .ArrayLayers(1)
        .MipLevels(1)
        .MemoryUsage(MEMORY_USAGE_GPU_ONLY)
        .AllowReadWrite()
        .AllowRenderTarget()
        .Finish();  

    RDGComputePassHandle pass = builder.CreateComputePass(GetName())
        .RootSignature(rootSignature)
        .ReadWrite(0, 0, 0, fxaaOutColor)
        .ReadWrite(0, 1, 0, outColor)
        .ReadWrite(0, 2, 0, exposureData)
        .Execute([&](RDGPassContext context) {       

            RHICommandListRef command = context.command; 
            command->SetComputePipeline(computePipeline);
            command->BindDescriptorSet(context.descriptors[0], 0);
            command->PushConstants(&setting, sizeof(PostProcessingSetting), SHADER_FREQUENCY_COMPUTE);
            command->Dispatch(  EngineContext::Render()->GetWindowsExtent().width / 16, 
                                EngineContext::Render()->GetWindowsExtent().height / 16, 
                                1);
        })
        .Finish();
}
