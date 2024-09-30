#pragma once

#include "RenderPass.h"

class FXAAPass : public RenderPass
{
public:
    FXAAPass() {};
	~FXAAPass() {};

	virtual void Init() override final;

	virtual void Build(RDGBuilder& builder) override final;

	virtual std::string GetName() override final { return "FXAA"; }

	virtual PassType GetType() override final { return FXAA_PASS; };

private:
	uint32_t enable = true;

    Shader computeShader;

    RHIRootSignatureRef rootSignature;
    RHIComputePipelineRef computePipeline;

	EnablePassEditourUI()
};