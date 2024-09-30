#include "ExposurePass.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RDG/RDGHandle.h"
#include "Function/Render/RHI/RHIStructs.h"
#include <cmath>

void ExposurePass::Init()
{
    auto backend = EngineContext::RHI();

    computeShader0 = Shader(EngineContext::File()->ShaderPath() + "post_process/luminance_histogram.comp.spv", SHADER_FREQUENCY_COMPUTE);
    computeShader1 = Shader(EngineContext::File()->ShaderPath() + "post_process/luminance_exposure.comp.spv", SHADER_FREQUENCY_COMPUTE);

    RHIRootSignatureInfo rootSignatureInfo = {};
    rootSignatureInfo.AddEntry({0, 0, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_TEXTURE})
                     .AddEntry({0, 1, 1, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_BUFFER});
    rootSignature = backend->CreateRootSignature(rootSignatureInfo);


    RHIComputePipelineInfo pipelineInfo     = {};
    pipelineInfo.computeShader              = computeShader0.shader;
    pipelineInfo.rootSignature              = rootSignature;
    computePipeline0                        = backend->CreateComputePipeline(pipelineInfo);

    pipelineInfo.computeShader              = computeShader1.shader;
    computePipeline1                        = backend->CreateComputePipeline(pipelineInfo);
}   

void ExposurePass::Build(RDGBuilder& builder) 
{
    Extent2D windowExtent = EngineContext::Render()->GetWindowsExtent();

    if(enableStatistics)    // 回读曝光度的数据
    {
        exposureDataBuffer.GetData(&readBackData);
    }
    ExposureSetting setting         = {};
    setting.minLog2Luminance        = log2(minLuminance);
    setting.luminanceRange          = log2(maxLuminance) - log2(minLuminance);
    setting.inverseLuminanceRange   = 1.0f / setting.luminanceRange;
    setting.numPixels               = windowExtent.width * windowExtent.height; 
    setting.timeCoeff               = adjustSpeed;
    exposureDataBuffer.SetData(&setting, sizeof(ExposureSetting), 0);


    RDGTextureHandle fxaaOutColor = builder.GetTexture("FXAA Out Color");
    
    RDGBufferHandle exposureData = builder.CreateBuffer("Exposure Data")
        .Import(exposureDataBuffer.buffer, RESOURCE_STATE_UNDEFINED)
        .Finish();

    RDGComputePassHandle pass0 = builder.CreateComputePass("Luminance Histogram")
        .RootSignature(rootSignature)
        .ReadWrite(0, 0, 0, fxaaOutColor)
        .ReadWrite(0, 1, 0, exposureData)
        .Execute([&](RDGPassContext context) {       

            RHICommandListRef command = context.command; 
            command->SetComputePipeline(computePipeline0);
            command->BindDescriptorSet(context.descriptors[0], 0);
            command->Dispatch(  EngineContext::Render()->GetWindowsExtent().width / 16, 
                                EngineContext::Render()->GetWindowsExtent().height / 16, 
                                1);
        })
        .Finish();

    RDGComputePassHandle pass1 = builder.CreateComputePass("Luminance Exposure")
        .RootSignature(rootSignature)
        .ReadWrite(0, 0, 0, fxaaOutColor)
        .ReadWrite(0, 1, 0, exposureData)
        .Execute([&](RDGPassContext context) {       

            RHICommandListRef command = context.command; 
            command->SetComputePipeline(computePipeline1);
            command->BindDescriptorSet(context.descriptors[0], 0);
            command->Dispatch(  1, 
                                1, 
                                1);
        })
        .Finish();
}
