#pragma once

#include "MeshPass.h"
#include "RenderPass.h"

class DirectionalShadowPass : public MeshPass
{
public:
    DirectionalShadowPass() {};
	~DirectionalShadowPass() {};

	virtual void Init() override final;

	virtual void Build(RDGBuilder& builder) override final;

	virtual std::string GetName() override final { return "Directional Shadow"; }

    virtual PassType GetType() override final { return DIRECTIONAL_SHADOW_PASS; };

    virtual std::vector<MeshPassProcessorRef> GetMeshPassProcessors() override final   { return meshPassProcessors; }   // 其实也可以检查，仅当需要更新时返回相应的processor

private:
    Shader vertexShader;
    Shader clusterVertexShader;
    Shader fragmentShader;

    RHIRootSignatureRef rootSignature;
    RHIGraphicsPipelineRef pipeline;
    RHIGraphicsPipelineRef clusterPipeline;

    friend class DirectionalShadowPassProcessor;

    std::vector<MeshPassProcessorRef> meshPassProcessors;

private:
	EnablePassEditourUI()
};

class DirectionalShadowPassProcessor : public MeshPassProcessor
{
public:
    DirectionalShadowPassProcessor(DirectionalShadowPass* pass) { this->pass = pass; }

    virtual void OnCollectBatch(const DrawBatch& batch) override final;                                                                              
    virtual RHIGraphicsPipelineRef OnCreatePipeline(const DrawPipelineState& pipelineState) override final;   

private:
    DirectionalShadowPass* pass;
};