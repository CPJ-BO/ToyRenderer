#include "RenderSystem.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RenderPass/GPUCullingPass.h"
#include "Function/Render/RenderPass/ClusterLightingPass.h"
#include "Function/Render/RenderPass/DepthPass.h"
#include "Function/Render/RenderPass/DepthPyramidPass.h"
#include "Function/Render/RenderPass/DirectionalShadowPass.h"
#include "Function/Render/RenderPass/PointShadowPass.h"
#include "Function/Render/RenderPass/GBufferPass.h"
#include "Function/Render/RenderPass/DeferredLightingPass.h"
#include "Function/Render/RenderPass/ForwardPass.h"
#include "Function/Render/RenderPass/BloomPass.h"
#include "Function/Render/RenderPass/FXAAPass.h"
#include "Function/Render/RenderPass/ExposurePass.h"
#include "Function/Render/RenderPass/PostProcessingPass.h"
#include "Function/Render/RenderPass/EditorUIPass.h"
#include "Function/Render/RenderPass/PresentPass.h"
#include "Function/Render/RenderPass/RenderPass.h"
#include <memory>

void RenderSystem::InitGLFW()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WINDOW_EXTENT.width, WINDOW_EXTENT.height, "Toy Renderer", nullptr, nullptr); 
    // glfwSetWindowUserPointer(m_window, this);
    // glfwSetWindowSizeCallback(m_window, nullptr); //TODO
    // glfwSetCursorPosCallback(m_window, MousePosCallback);
    // glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
    // glfwSetScrollCallback(m_window, ScrollCallback);
    // glfwSetKeyCallback(m_window, KeyCallback);
}

void RenderSystem::DestroyGLFW()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void RenderSystem::Init() 
{ 
    EngineContext::RHI()->InitImGui(window);

    lightManager = std::make_shared<RenderLightManager>();
    meshManager = std::make_shared<RenderMeshManager>();
    lightManager->Init();
    meshManager->Init();

    InitBaseResource(); 
    InitPasses();  
}

void RenderSystem::InitBaseResource()
{
    backend       = EngineContext::RHI();
    surface       = backend->CreateSurface(window);
    queue         = backend->GetQueue({ QUEUE_TYPE_GRAPHICS, 0 });
    swapchain     = backend->CreateSwapChain({ surface, queue, FRAMES_IN_FLIGHT, surface->GetExetent(), COLOR_FORMAT });
    pool          = backend->CreateCommandPool({ queue });  
    for(uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++) 
    {
        perFrameCommonResources[i].command = pool->CreateCommandList(false);
        perFrameCommonResources[i].startSemaphore = backend->CreateSemaphore();
        perFrameCommonResources[i].finishSemaphore = backend->CreateSemaphore();
        perFrameCommonResources[i].fence = backend->CreateFence(true);
    }
}

void RenderSystem::InitPasses()
{
    meshPasses[MESH_DEPTH_PASS]                 = std::make_shared<DepthPass>();
    meshPasses[MESH_DIRECTIONAL_SHADOW_PASS]    = std::make_shared<DirectionalShadowPass>();
    meshPasses[MESH_POINT_SHADOW_PASS]          = std::make_shared<PointShadowPass>();
    meshPasses[MESH_G_BUFFER_PASS]              = std::make_shared<GBufferPass>();
    meshPasses[MESH_FORWARD_PASS]               = std::make_shared<ForwardPass>();

    passes[GPU_CULLING_PASS]                    = std::make_shared<GPUCullingPass>();
    passes[CLUSTER_LIGHTING_PASS]               = std::make_shared<ClusterLightingPass>();
    passes[DEPTH_PASS]                          = meshPasses[MESH_DEPTH_PASS];
    passes[DEPTH_PYRAMID_PASS]                  = std::make_shared<DepthPyramidPass>();
    passes[POINT_SHADOW_PASS]                   = meshPasses[MESH_POINT_SHADOW_PASS];
    passes[DIRECTIONAL_SHADOW_PASS]             = meshPasses[MESH_DIRECTIONAL_SHADOW_PASS];
    passes[G_BUFFER_PASS]                       = meshPasses[MESH_G_BUFFER_PASS];
    passes[DEFERRED_LIGHTING_PASS]              = std::make_shared<DeferredLightingPass>();
    passes[FORWARD_PASS]                        = meshPasses[MESH_FORWARD_PASS];
    passes[TRANSPARENT_PASS]                    = nullptr;
    passes[BLOOM_PASS]                          = std::make_shared<BloomPass>();
    passes[FXAA_PASS]                           = std::make_shared<FXAAPass>();
    passes[EXPOSURE_PASS]                       = std::make_shared<ExposurePass>();
    passes[POST_PROCESSING_PASS]                = std::make_shared<PostProcessingPass>();
    passes[EDITOR_UI_PASS]                      = std::make_shared<EditorUIPass>();
    passes[PRESENT_PASS]                        = std::make_shared<PresentPass>();

    for(auto& pass : passes) { if(pass) pass->Init(); }
}

bool RenderSystem::Tick()
{
    ENGINE_TIME_SCOPE(RenderSystem::Tick);
    if(!glfwWindowShouldClose(window))
    {
        if(EngineContext::World()->GetActiveScene() == nullptr) return false;

        meshManager->Tick();    // 先准备各个meshpass的绘制信息
        lightManager->Tick();   // 准备光源信息   
        UpdateGlobalSetting();                   

        auto& resource = perFrameCommonResources[EngineContext::CurrentFrameIndex()];

        resource.fence->Wait();                         // 等待帧栅栏
        RHITextureRef swapchainTexture = swapchain->GetNewFrame(nullptr, resource.startSemaphore); 
        RHICommandListRef command = resource.command;   // 构建RDG，绘制提交
        command->BeginCommand();
        {
            ENGINE_TIME_SCOPE(RenderSystem::RDGBuild);
            RDGBuilder rdgBuilder = RDGBuilder(command);
            
            for(auto& pass : passes) { if(pass) pass->Build(rdgBuilder); }
            rdgBuilder.Execute();
        }
        {
            ENGINE_TIME_SCOPE(RenderSystem::RecordCommands);
            command->EndCommand();
            command->Execute(resource.fence, resource.startSemaphore, resource.finishSemaphore);
        }      
        swapchain->Present(resource.finishSemaphore); 
        return false; 
    }
    return true;
}

void RenderSystem::UpdateGlobalSetting()
{
    globalSetting.totalTicks++;
    globalSetting.totalTickTime += EngineContext::GetDeltaTime();
    globalSetting.skyboxMaterialID = EngineContext::World()->GetActiveScene()->GetSkyBox() ?
                                     EngineContext::World()->GetActiveScene()->GetSkyBox()->GetMaterialID() : 0;
    //globalSetting.clusterInspectMode;   // 在Editor里设置
    EngineContext::RenderResource()->SetRenderGlobalSetting(globalSetting);
}

