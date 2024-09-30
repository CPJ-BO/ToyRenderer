#pragma once

#include "RenderPass.h"

class DeferredLightingPass : public RenderPass
{
public:
	DeferredLightingPass() = default;
	~DeferredLightingPass() {};

	virtual void Init() override final;

	virtual void Build(RDGBuilder& builder) override final;

	virtual std::string GetName() override final { return "Deferred Lighting"; }

	virtual PassType GetType() override final { return DEFERRED_LIGHTING_PASS; };

private:
	struct DeferredLightingSetting{
		uint32_t mode = 0;
	};
	DeferredLightingSetting setting = {};

    Shader computeShader;

    RHIRootSignatureRef rootSignature;
    RHIComputePipelineRef computePipeline;

private:
	EnablePassEditourUI()
};