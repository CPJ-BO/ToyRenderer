#include "PointShadowPass.h"
#include "Function/Global/Definations.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "Function/Render/RenderResource/PipelineCache.h"
#include <cstdint>

void PointShadowPassProcessor::OnCollectBatch(const DrawBatch& batch)
{
    if(batch.material->CastShadow()) AddBatch(batch);
}

RHIGraphicsPipelineRef PointShadowPassProcessor::OnCreatePipeline(const DrawPipelineState& pipelineState)
{
    if(pipelineState.clusterRender) return pass->clusterPipeline;                       // 用默认管线
    else                            return pass->pipeline;                                                      
}

void PointShadowPass::Init()
{
    for(uint32_t i = 0; i < MAX_POINT_SHADOW_COUNT; i++)
    {
        auto processor = std::make_shared<PointShadowPassProcessor>(this);
        processor->Init();
        meshPassProcessors.push_back(processor);
    }

    auto backend = EngineContext::RHI();

    // pass0 ///////////////////////////////////////////////////////////////////////////////
    {
        vertexShader        = Shader(EngineContext::File()->ShaderPath() + "shadow/point_shadow.vert.spv", SHADER_FREQUENCY_VERTEX);
        clusterVertexShader = Shader(EngineContext::File()->ShaderPath() + "shadow/point_shadow_cluster.vert.spv", SHADER_FREQUENCY_VERTEX);
        geometryShader      = Shader(EngineContext::File()->ShaderPath() + "shadow/point_shadow.geom.spv", SHADER_FREQUENCY_GEOMETRY);
        fragmentShader      = Shader(EngineContext::File()->ShaderPath() + "shadow/point_shadow.frag.spv", SHADER_FREQUENCY_FRAGMENT);

        RHIRootSignatureInfo rootSignatureInfo = {};
        rootSignatureInfo.AddEntry(EngineContext::RenderResource()->GetPerFrameRootSignature()->GetInfo())
                         .AddPushConstant({128, SHADER_FREQUENCY_GRAPHICS});
        rootSignature0 = backend->CreateRootSignature(rootSignatureInfo);

        RHIGraphicsPipelineInfo pipelineInfo = {};
        pipelineInfo.vertexShader       = vertexShader.shader;
        pipelineInfo.geometryShader     = geometryShader.shader;
        pipelineInfo.fragmentShader     = fragmentShader.shader;
        pipelineInfo.rootSignature      = rootSignature0;
        pipelineInfo.primitiveType      = PRIMITIVE_TYPE_TRIANGLE_LIST;
        pipelineInfo.rasterizerState    = { FILL_MODE_SOLID, CULL_MODE_NONE, DEPTH_CLIP, 0.0f, 0.0f };  // 不要做背面剔除，双面阴影 
        pipelineInfo.blendState.renderTargets[0].enable = false;
        pipelineInfo.colorAttachmentFormats[0]      = FORMAT_R32G32B32A32_SFLOAT;                   
        pipelineInfo.depthStencilState              = { COMPARE_FUNCTION_LESS_EQUAL, true, true };
        pipelineInfo.depthStencilAttachmentFormat   = FORMAT_D32_SFLOAT;
        pipeline                                    = GraphicsPipelineCache::Get()->Allocate(pipelineInfo).pipeline;    // 普通mesh的默认绘制管线

        pipelineInfo.vertexShader                   = clusterVertexShader.shader;
        clusterPipeline                             = GraphicsPipelineCache::Get()->Allocate(pipelineInfo).pipeline;    // cluster的默认绘制管线
    }

    // pass1 ///////////////////////////////////////////////////////////////////////////////
    {
        computeShader    = Shader(EngineContext::File()->ShaderPath() + "shadow/point_shadow_kawase.comp.spv", SHADER_FREQUENCY_COMPUTE);

        RHIRootSignatureInfo rootSignatureInfo = {};
        rootSignatureInfo.AddEntry({0, 0, 6, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_TEXTURE})    
                         .AddEntry({0, 1, 6, SHADER_FREQUENCY_COMPUTE, RESOURCE_TYPE_RW_TEXTURE}) 
                         .AddEntry(EngineContext::RenderResource()->GetSamplerRootSignature()->GetInfo()) 
                         .AddPushConstant({128, SHADER_FREQUENCY_COMPUTE});                                 
        rootSignature1 = backend->CreateRootSignature(rootSignatureInfo);

        RHIComputePipelineInfo pipelineInfo     = {};
        pipelineInfo.computeShader              = computeShader.shader;
        pipelineInfo.rootSignature              = rootSignature1;
        computePipeline                         = backend->CreateComputePipeline(pipelineInfo);
    }

}   

void PointShadowPass::Build(RDGBuilder& builder) 
{
    pointShadowLights = EngineContext::Render()->GetLightManager()->GetPointShadowLights();
    for(uint32_t i = 0; i < pointShadowLights.size(); i++)
    {
        std::string index = " [" + std::to_string(i) + "]";

        RDGTextureHandle color = builder.CreateTexture("Point Shadow Color" + index)
            .Exetent({POINT_SHADOW_SIZE, POINT_SHADOW_SIZE, 1})
            .Format(FORMAT_R32G32B32A32_SFLOAT)
            .ArrayLayers(6)
            .MipLevels(1)
            .AllowRenderTarget()
            .Finish();

        RDGTextureHandle filteredColor = builder.CreateTexture("Point Shadow Filtered Color" + index)
            .Import(EngineContext::RenderResource()->GetPointShadowTexture(i), RESOURCE_STATE_UNDEFINED)
            .Finish();

        RDGTextureHandle depth = builder.CreateTexture("Point Shadow Depth" + index)
            .Exetent({POINT_SHADOW_SIZE, POINT_SHADOW_SIZE, 1})
            .Format(FORMAT_D32_SFLOAT)
            .ArrayLayers(6)
            .MipLevels(1)
            .AllowDepthStencil()
            .Finish();

        RDGRenderPassHandle pass = builder.CreateRenderPass(GetName() + index)
            .PassIndex(i)
            .Color(0, color, ATTACHMENT_LOAD_OP_CLEAR, ATTACHMENT_STORE_OP_STORE, 
                {0.0f, 0.0f, 0.0f, 0.0f}, {TEXTURE_ASPECT_COLOR, 0, 1, 0, 6})
            .DepthStencil(depth, ATTACHMENT_LOAD_OP_CLEAR, ATTACHMENT_STORE_OP_STORE, 
                1.0f, 0, {TEXTURE_ASPECT_DEPTH, 0, 1, 0, 6})
            .Execute([&](RDGPassContext context) {

                Extent2D windowExtent = EngineContext::Render()->GetWindowsExtent();
                uint32_t index = context.passIndex;
                uint32_t pointLightID = pointShadowLights[index]->GetPointLightID();

                RHICommandListRef command = context.command;      
                command->SetGraphicsPipeline(pipeline);                                      
                command->SetViewport({0, 0}, {POINT_SHADOW_SIZE, POINT_SHADOW_SIZE});
                command->SetScissor({0, 0}, {POINT_SHADOW_SIZE, POINT_SHADOW_SIZE}); 
                command->SetDepthBias(pointShadowLights[index]->GetConstantBias(), 
                                        pointShadowLights[index]->GetSlopeBias(), 
                                        0.0f);            
                command->PushConstants(&pointLightID, sizeof(uint32_t), SHADER_FREQUENCY_GRAPHICS);
                command->BindDescriptorSet(EngineContext::RenderResource()->GetPerFrameDescriptorSet(), 0);   

                meshPassProcessors[index]->Draw(command);                      
            })
            .OutputRead(color)  // 手动屏障
            .Finish();

        RDGComputePassHandle filterPass = builder.CreateComputePass(GetName() + " Filter" + index)  //TODO 多轮？
            .RootSignature(rootSignature1)
            .Read(0, 0, 0, color, VIEW_TYPE_2D, {TEXTURE_ASPECT_COLOR, 0, 1, 0, 1})
            .Read(0, 0, 1, color, VIEW_TYPE_2D, {TEXTURE_ASPECT_COLOR, 0, 1, 1, 1})
            .Read(0, 0, 2, color, VIEW_TYPE_2D, {TEXTURE_ASPECT_COLOR, 0, 1, 2, 1})
            .Read(0, 0, 3, color, VIEW_TYPE_2D, {TEXTURE_ASPECT_COLOR, 0, 1, 3, 1})
            .Read(0, 0, 4, color, VIEW_TYPE_2D, {TEXTURE_ASPECT_COLOR, 0, 1, 4, 1})
            .Read(0, 0, 5, color, VIEW_TYPE_2D, {TEXTURE_ASPECT_COLOR, 0, 1, 5, 1})
            .ReadWrite(0, 1, 0, filteredColor, VIEW_TYPE_2D, {TEXTURE_ASPECT_COLOR, 0, 1, 0, 1})
            .ReadWrite(0, 1, 1, filteredColor, VIEW_TYPE_2D, {TEXTURE_ASPECT_COLOR, 0, 1, 1, 1})
            .ReadWrite(0, 1, 2, filteredColor, VIEW_TYPE_2D, {TEXTURE_ASPECT_COLOR, 0, 1, 2, 1})
            .ReadWrite(0, 1, 3, filteredColor, VIEW_TYPE_2D, {TEXTURE_ASPECT_COLOR, 0, 1, 3, 1})
            .ReadWrite(0, 1, 4, filteredColor, VIEW_TYPE_2D, {TEXTURE_ASPECT_COLOR, 0, 1, 4, 1})
            .ReadWrite(0, 1, 5, filteredColor, VIEW_TYPE_2D, {TEXTURE_ASPECT_COLOR, 0, 1, 5, 1})  
            .Execute([&](RDGPassContext context) {       

                RHICommandListRef command = context.command; 
                command->SetComputePipeline(computePipeline);
                command->BindDescriptorSet(context.descriptors[0], 0);
                command->BindDescriptorSet(EngineContext::RenderResource()->GetSamplerDescriptorSet(), 1);
                command->PushConstants(&setting, sizeof(KawaseSetting), SHADER_FREQUENCY_COMPUTE);
                command->Dispatch(  POINT_SHADOW_SIZE / 16, 
                                    POINT_SHADOW_SIZE / 16, 
                                    1);
            })
            .OutputRead(filteredColor)
            .Finish();
    }
}
