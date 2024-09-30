#pragma once

#include "RenderPass.h"

class BloomPass : public RenderPass
{
public:
    BloomPass() {};
	~BloomPass() {};

	virtual void Init() override final;

	virtual void Build(RDGBuilder& builder) override final;

	virtual std::string GetName() override final { return "Bloom"; }

	virtual PassType GetType() override final { return BLOOM_PASS; };

private:
	struct BloomSetting {
		int maxMip = 0;
		int mipLevel = 0;
		float stride = 1.0f;
		float bias = 6.0f;
		float accumulateIntencity = 2.0f;
		float combineIntencity = 0.1f;
	};
	BloomSetting setting;

    Shader computeShader0;
	Shader computeShader1;
	Shader computeShader2;
	Shader computeShader3;

    RHIComputePipelineRef computePipeline0;
	RHIComputePipelineRef computePipeline1;
	RHIComputePipelineRef computePipeline2;
	RHIComputePipelineRef computePipeline3;

	RHIRootSignatureRef rootSignature;

	EnablePassEditourUI()
};