#include "EngineContext.h"
#include "Core/Log/LogSystem.h"
#include "Function/Framework/World/WorldManager.h"
#include "Function/Global/Definations.h"
#include "Function/Render/RHI/RHI.h"
#include "Function/Render/RenderResource/RenderResourceManager.h"
#include "Platform/File/FileSystem.h"
#include "Platform/Thread/QueuedThreadPool.h"
#include <memory>

std::shared_ptr<EngineContext> EngineContext::context = std::make_shared<EngineContext>();

std::shared_ptr<EngineContext> EngineContext::Init()
{  
    context->logSystem = std::make_shared<LogSystem>();
    context->logSystem->Init();

    context->threadPool = QueuedThreadPool::Create(10);

    context->rhiBackend = RHIBackend::Init({.type = BACKEND_VULKAN, .enableDebug = true, .enableRayTracing = false});

    context->fileSystem = std::make_shared<FileSystem>();
    context->fileSystem->Init("renderer");

    context->assetManager = std::make_shared<AssetManager>();
    context->assetManager->Init();

    context->worldManager = std::make_shared<WorldManager>(); 
    //context->worldManager->Init("resource/build_in/config/scene/default.scene");

    context->renderResourceManger = std::make_shared<RenderResourceManager>();
    context->renderResourceManger->Init();

    return context;
}

void EngineContext::TickInternal()
{
    timer.End();
    deltaTime = timer.GetMilliSeconds();

    rhiBackend->Tick();
    worldManager->Tick(deltaTime);

    currentFrameIndex = (currentFrameIndex + 1) % FRAMES_IN_FLIGHT;
    timer.Clear();
    timer.Begin();
}

void EngineContext::DestroyInternal()
{
    ENGINE_LOG_INFO("Engine context destructed.");
    renderResourceManger->Destroy();
    assetManager->Save();
    rhiBackend->Destroy();
    threadPool->Destroy();
    logSystem->Destroy();
}