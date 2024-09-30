#pragma once

#include "Function/Render/RenderResource/Shader.h"
#include "MeshPass.h"
#include "RenderPass.h"

class GBufferPass : public MeshPass
{
public:
    GBufferPass() {};
	~GBufferPass() {};

	virtual void Init() override final;

	virtual void Build(RDGBuilder& builder) override final;

	virtual std::string GetName() override final { return "G-Buffer"; }

    virtual PassType GetType() override final { return G_BUFFER_PASS; };

private:
    Shader vertexShader;
    Shader clusterVertexShader;
    Shader fragmentShader;
    RHIBufferRef settingBuffer;

    RHIRootSignatureRef rootSignature;
    RHIGraphicsPipelineRef pipeline;
    RHIGraphicsPipelineRef clusterPipeline;

    friend class GBufferPassProcessor;

private:
	EnablePassEditourUI()
};

class GBufferPassProcessor : public MeshPassProcessor
{
public:
    GBufferPassProcessor(GBufferPass* pass) { this->pass = pass; }

    virtual void OnCollectBatch(const DrawBatch& batch) override final;                                       
    // virtual void OnBuildDrawInfo(const DrawBatch& batch) override final;                                       
    virtual RHIGraphicsPipelineRef OnCreatePipeline(const DrawPipelineState& pipelineState) override final;   
    // virtual void OnBuildDrawCommands(                                                           
    //                 uint32_t pipelineIndex,     
    //                 RHIGraphicsPipelineRef pipeline, 
    //                 const std::vector<DrawGeometryInfo>& geometries) override final;

private:
    GBufferPass* pass;
};