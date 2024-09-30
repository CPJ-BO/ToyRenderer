#pragma once

#include "RenderPass.h"

class ExposurePass : public RenderPass
{
public:
    ExposurePass() {};
	~ExposurePass() {};

	virtual void Init() override final;

	virtual void Build(RDGBuilder& builder) override final;

	virtual std::string GetName() override final { return "Exposure"; }

	virtual PassType GetType() override final { return EXPOSURE_PASS; };

private:
    bool enableStatistics = false;

    float minLuminance = 1.0f / 128.0f;
    float maxLuminance = 4.0f;
    float adjustSpeed = 0.01f;

    struct ExposureSetting
    {
        float minLog2Luminance = 0.0f;       
        float inverseLuminanceRange = 0.0f;  
        float luminanceRange = 0.0f;         
        float numPixels = 0.0f;             
        float timeCoeff = 0.0f;               
        float _padding[3];  
    };
    struct ExposureData
    {
        ExposureSetting setting = {};         
        float luminance = 0.0f;
        float adaptedLuminance = 0.0f;
        float _padding[2];
        uint32_t histogramBuffer[256] = { 0 };      
        uint32_t readBackHistogramBuffer[256] = { 0 };   
    };
    Buffer<ExposureData> exposureDataBuffer;
    ExposureData readBackData = {};

    Shader computeShader0;
    Shader computeShader1;

    RHIRootSignatureRef rootSignature;
    RHIComputePipelineRef computePipeline0;
    RHIComputePipelineRef computePipeline1;

	EnablePassEditourUI()
};