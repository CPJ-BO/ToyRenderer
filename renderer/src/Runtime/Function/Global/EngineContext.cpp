#include "EngineContext.h"
#include "Core/Log/LogSystem.h"
#include "Core/Util/TimeScope.h"
#include "Function/Framework/World/WorldManager.h"
#include "Function/Global/Definations.h"
#include "Function/Render/RHI/RHI.h"
#include "Function/Render/RenderResource/RenderResourceManager.h"
#include "Platform/File/FileSystem.h"
#include "Platform/HAL/PlatformProcess.h"
#include "Platform/Thread/QueuedThreadPool.h"
#include <memory>

std::shared_ptr<EngineContext> EngineContext::context = std::make_shared<EngineContext>();

std::shared_ptr<EngineContext> EngineContext::Init()
{  
    for(auto& timer : context->timers) timer = std::make_shared<TimeScopes>();

    context->logSystem = std::make_shared<LogSystem>();
    context->logSystem->Init();

    context->fileSystem = std::make_shared<FileSystem>();
    context->fileSystem->Init("renderer");

    context->renderSystem = std::make_shared<RenderSystem>();
    context->renderSystem->InitGLFW();

    context->inputSystem = std::make_shared<InputSystem>();
    context->inputSystem->Init();
    context->inputSystem->InitGLFW();

    context->threadPool = QueuedThreadPool::Create(10);

    context->rhiBackend = RHIBackend::Init({.type = BACKEND_VULKAN, .enableDebug = true, .enableRayTracing = false});

    context->renderResourceManger = std::make_shared<RenderResourceManager>();
    context->renderResourceManger->Init();

    context->renderSystem->Init();

    context->editorSystem = std::make_shared<EditorSystem>();
    context->editorSystem->Init();

    context->worldManager = std::make_shared<WorldManager>(); 
    //context->worldManager->Init("resource/build_in/config/scene/default.scene");

    context->assetManager = std::make_shared<AssetManager>();
    context->assetManager->Init();

    return context;
}

void EngineContext::MainLoopInternal()
{
    bool exit = false;
    while (!exit) 
    {
        timers[currentFrameIndex] = std::make_shared<TimeScopes>(); // 重新生成对象，不clear了
        ENGINE_TIME_SCOPE(EngineContext::MainLoopInternal);
        
        timer.EndAfterMilliSeconds(renderSystem->GetGlobalSetting()->minFrameTime);
        deltaTime = timer.GetMilliSeconds();
        timer.Clear();
        timer.Begin();         

        inputSystem->Tick();
        rhiBackend->Tick();
        worldManager->Tick(deltaTime);
        assetManager->Tick();
        exit = renderSystem->Tick();

        currentFrameIndex = (currentFrameIndex + 1) % FRAMES_IN_FLIGHT;
    }
}

void EngineContext::DestroyInternal()
{
    ENGINE_LOG_INFO("Engine context destructed.");
    renderResourceManger->Destroy();
    renderSystem->Destroy();
    worldManager->Save();
    assetManager->Save();
    rhiBackend->Destroy();
    threadPool->Destroy();
    logSystem->Destroy();
    renderSystem->DestroyGLFW();
}