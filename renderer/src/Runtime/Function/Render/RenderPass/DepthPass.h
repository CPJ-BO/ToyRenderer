#pragma once

#include "Function/Render/RenderPass/RenderPass.h"
#include "MeshPass.h"
#include "RenderPass.h"

class DepthPass : public MeshPass
{
public:
    DepthPass() {};
	~DepthPass() {};

	virtual void Init() override final;

	virtual void Build(RDGBuilder& builder) override final;

	virtual std::string GetName() override final { return "Depth"; }

    virtual PassType GetType() override final { return DEPTH_PASS; };

private:
    Shader vertexShader;
    Shader clusterVertexShader;
    Shader fragmentShader;

    RHIRootSignatureRef rootSignature;
    RHIGraphicsPipelineRef pipeline;
    RHIGraphicsPipelineRef clusterPipeline;

    friend class DepthPassProcessor;

private:
	EnablePassEditourUI()
};

class DepthPassProcessor : public MeshPassProcessor
{
public:
    DepthPassProcessor(DepthPass* pass) { this->pass = pass; }

    virtual void OnCollectBatch(const DrawBatch& batch) override final;                                                                              
    virtual RHIGraphicsPipelineRef OnCreatePipeline(const DrawPipelineState& pipelineState) override final;   

private:
    DepthPass* pass;
};