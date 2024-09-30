#pragma once

#include "RenderPass.h"

class ClusterLightingPass : public RenderPass
{
public:
    ClusterLightingPass() {};
	~ClusterLightingPass() {};

	virtual void Init() override final;

	virtual void Build(RDGBuilder& builder) override final;

	virtual std::string GetName() override final { return "Cluster Based Lighting"; }

	virtual PassType GetType() override final { return CLUSTER_LIGHTING_PASS; };

private:
    Shader computeShader;

    RHIRootSignatureRef rootSignature;
    RHIComputePipelineRef computePipeline;

	EnablePassEditourUI()
};
