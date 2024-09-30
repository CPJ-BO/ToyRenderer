#pragma once

#include "RenderPass.h"
#include <cstdint>

class DepthPyramidPass : public RenderPass
{
public:
    DepthPyramidPass() {};
	~DepthPyramidPass() {};

	virtual void Init() override final;

	virtual void Build(RDGBuilder& builder) override final;

	virtual std::string GetName() override final { return "Depth Pyramid"; }

	virtual PassType GetType() override final { return DEPTH_PYRAMID_PASS; };

private:
	struct DepthPyramidSetting
	{
		uint32_t mode = 0;
		uint32_t mipLevel = 0;
	};

    Shader computeShader;

    RHIRootSignatureRef rootSignature;
    RHIComputePipelineRef computePipeline;

	void Build(RDGBuilder& builder, uint32_t mode, RDGTextureHandle depthMip);

	EnablePassEditourUI()
};