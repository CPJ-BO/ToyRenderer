#pragma once

#include "MeshPass.h"
#include "RenderPass.h"

class ForwardPass : public MeshPass
{
public:
    ForwardPass() {};
	~ForwardPass() {};

	virtual void Init() override final;

	virtual void Build(RDGBuilder& builder) override final;

	virtual std::string GetName() override final { return "Forward"; }

    virtual PassType GetType() override final { return FORWARD_PASS; };

private:
    Shader vertexShader;
    Shader clusterVertexShader;
    Shader fragmentShader;

    RHIRootSignatureRef rootSignature;
    RHIGraphicsPipelineRef pipeline;
    RHIGraphicsPipelineRef clusterPipeline;

    friend class ForwardPassProcessor;

private:
	EnablePassEditourUI()
};

class ForwardPassProcessor : public MeshPassProcessor
{
public:
    ForwardPassProcessor(ForwardPass* pass) { this->pass = pass; }

    virtual void OnCollectBatch(const DrawBatch& batch) override final;                                       
    // virtual void OnBuildDrawInfo(const DrawBatch& batch) override final;                                       
    virtual RHIGraphicsPipelineRef OnCreatePipeline(const DrawPipelineState& pipelineState) override final;   
    // virtual void OnBuildDrawCommands(                                                           
    //                 uint32_t pipelineIndex,     
    //                 RHIGraphicsPipelineRef pipeline, 
    //                 const std::vector<DrawGeometryInfo>& geometries) override final;

private:
    ForwardPass* pass;
};