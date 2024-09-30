#pragma once

#include "Function/Render/RenderResource/Buffer.h"
#include "Function/Render/RenderResource/RenderStructs.h"

#include "MeshPass.h"
#include "RenderPass.h"
#include <array>
#include <cstdint>
#include <vector>

class GPUCullingPass : public RenderPass
{
public:
	GPUCullingPass() = default;
	~GPUCullingPass() {};

	virtual void Init() override final;

	virtual void Build(RDGBuilder& builder) override final;

	virtual std::string GetName() override final { return "GPU Culling"; }

	virtual PassType GetType() override final { return GPU_CULLING_PASS; };

	void CollectStatisticDatas();

private:
	struct CullingSetting
	{
		uint32_t processSize = 0;
		uint32_t _padding[3];

		uint32_t passType [MAX_SUPPORTED_MESH_PASS_COUNT];
		uint32_t passOffset [MAX_SUPPORTED_MESH_PASS_COUNT];
	};
	Buffer<CullingSetting> cullingSettingBuffer;
	CullingSetting cullingSetting;

	struct CullingLodSetting
	{
		float lodErrorRate = 0.01f;
		float lodErrorOffset = 0.0f;
	};
	CullingLodSetting lodSetting = {};

    Shader cullingShader;
	Shader clusterCullingShader;
    
    RHIRootSignatureRef rootSignature;
    RHIComputePipelineRef cullingPipeline;
	RHIComputePipelineRef clusterCullingPipeline;

	std::array<std::vector<std::shared_ptr<MeshPassIndirectBuffers>>, MESH_PASS_TYPE_MAX_CNT> indirectBuffers;   

private:
	struct ReadBackData
	{
		IndirectSetting meshSetting;
		IndirectSetting clusterSetting;
		IndirectSetting clusterGroupSetting;
	};

	bool enableStatistics = false;
	std::array<std::vector<ReadBackData>, MESH_PASS_TYPE_MAX_CNT> readBackDatas;   

	EnablePassEditourUI()
};