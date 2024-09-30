#pragma once

#include "Function/Global/Definations.h"
#include "Function/Render/RenderPass/MeshPass.h"
#include "Function/Render/RenderPass/RenderPass.h"
#include "RenderLightManager.h"
#include "RenderMeshManager.h"

#include <array>
#include <memory>


static const Extent2D WINDOW_EXTENT = { WINDOW_WIDTH, WINDOW_HEIGHT };
static const RHIFormat HDR_COLOR_FORMAT = FORMAT_R16G16B16A16_SFLOAT;
static const RHIFormat COLOR_FORMAT = FORMAT_R8G8B8A8_UNORM;   
static const RHIFormat DEPTH_FORMAT = FORMAT_D32_SFLOAT;

class RenderSystem
{
public:
    void Init();
    void Destroy() {}
    void InitGLFW();
    void DestroyGLFW();

    bool Tick();

    const std::array<std::shared_ptr<RenderPass>, PASS_TYPE_MAX_CNT>& GetPasses()           { return passes; }
    const std::array<std::shared_ptr<MeshPass>, MESH_PASS_TYPE_MAX_CNT>& GetMeshPasses()    { return meshPasses; }

    GLFWwindow* GetWindow()         { return window; }
    Extent2D GetWindowsExtent()     { return WINDOW_EXTENT; }
    RHIFormat GetHdrColorFormat()   { return HDR_COLOR_FORMAT; } 
    RHIFormat GetColorFormat()      { return COLOR_FORMAT; } 
    RHIFormat GetDepthFormat()      { return DEPTH_FORMAT; }
    RHISwapchainRef GetSwapchain()  { return swapchain; }

    inline std::shared_ptr<RenderMeshManager> GetMeshManager()      { return meshManager; }
    inline std::shared_ptr<RenderLightManager> GetLightManager()    { return lightManager; }
    RenderGlobalSetting* GetGlobalSetting()                         { return &globalSetting; }

private:
    GLFWwindow* window;

    RHIBackendRef backend;
    RHISurfaceRef surface;
    RHIQueueRef queue;
    RHISwapchainRef swapchain;
    RHICommandPoolRef pool;
    

    struct PerFrameCommonResource 
    {
        RHICommandListRef command;
        RHISemaphoreRef startSemaphore;
        RHISemaphoreRef finishSemaphore;
        RHIFenceRef fence;
    };
    std::array<PerFrameCommonResource, FRAMES_IN_FLIGHT> perFrameCommonResources;
    RenderGlobalSetting globalSetting = {};

    std::array<std::shared_ptr<RenderPass>, PASS_TYPE_MAX_CNT> passes;
    std::array<std::shared_ptr<MeshPass>, MESH_PASS_TYPE_MAX_CNT> meshPasses;

    void InitPasses();
    void InitBaseResource();
    void UpdateGlobalSetting();

    std::shared_ptr<RenderMeshManager> meshManager;
    std::shared_ptr<RenderLightManager> lightManager;
};