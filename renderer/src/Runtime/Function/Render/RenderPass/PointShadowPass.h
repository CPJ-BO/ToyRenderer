#pragma once

#include "Function/Framework/Component/PointLightComponent.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "MeshPass.h"
#include "RenderPass.h"
#include <memory>
#include <vector>

class PointShadowPass : public MeshPass
{
public:
    PointShadowPass() {};
	~PointShadowPass() {};

	virtual void Init() override final;

	virtual void Build(RDGBuilder& builder) override final;

	virtual std::string GetName() override final { return "Point Shadow"; }

    virtual PassType GetType() override final { return POINT_SHADOW_PASS; };

    virtual std::vector<MeshPassProcessorRef> GetMeshPassProcessors() override final   { return meshPassProcessors; } 

private:
    // pass0 阴影
    Shader vertexShader;
    Shader clusterVertexShader;
    Shader geometryShader;
    Shader fragmentShader;

    RHIRootSignatureRef rootSignature0;
    RHIGraphicsPipelineRef pipeline;
    RHIGraphicsPipelineRef clusterPipeline;

    // pass1 滤波
	struct KawaseSetting
	{
		uint32_t round = 0;
	};
	KawaseSetting setting;

    Shader computeShader;

    RHIRootSignatureRef rootSignature1;
    RHIComputePipelineRef computePipeline;



    friend class PointShadowPassProcessor;

    std::vector<MeshPassProcessorRef> meshPassProcessors;
    std::vector<std::shared_ptr<PointLightComponent>> pointShadowLights;

private:
	EnablePassEditourUI()
};

class PointShadowPassProcessor : public MeshPassProcessor
{
public:
    PointShadowPassProcessor(PointShadowPass* pass) { this->pass = pass; }

    virtual void OnCollectBatch(const DrawBatch& batch) override final;                                                                              
    virtual RHIGraphicsPipelineRef OnCreatePipeline(const DrawPipelineState& pipelineState) override final;   

private:
    PointShadowPass* pass;
};