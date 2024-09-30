#pragma once

#include "Function/Render/RenderPass/BloomPass.h"
#include "Function/Render/RenderPass/PostProcessingPass.h"
#include "Function/Render/RenderPass/DeferredLightingPass.h"
#include "Function/Render/RenderPass/FXAAPass.h"
#include "Function/Render/RenderPass/GBufferPass.h"
#include "Function/Render/RenderPass/GPUCullingPass.h"
#include "Function/Render/RenderPass/ExposurePass.h"
#include "Function/Render/RenderPass/RenderPass.h"

#include <memory>

class PassWidget
{
public:
    static void UI(std::shared_ptr<RenderPass> pass);

private:
    static void GPUCullingPassUI(std::shared_ptr<GPUCullingPass> pass);
    static void GBufferPassUI(std::shared_ptr<GBufferPass> pass);
    static void DeferredLightingPassUI(std::shared_ptr<DeferredLightingPass> pass);
    static void BloomPassUI(std::shared_ptr<BloomPass> pass);
    static void FXAAPassUI(std::shared_ptr<FXAAPass> pass);
    static void ExposurePassUI(std::shared_ptr<ExposurePass> pass);
    static void PostProcessingPassUI(std::shared_ptr<PostProcessingPass> pass);  
};