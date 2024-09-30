#pragma once

#include "Core/Log/LogSystem.h"
#include "Core/Util/TimeScope.h"
#include "Function/Framework/World/WorldManager.h"
#include "Function/Render/RenderResource/RenderResourceManager.h"
#include "Function/Render/RenderSystem/RenderSystem.h"
#include "Platform/File/FileSystem.h"
#include "Function/Render/RHI/RHI.h"
#include "Platform/Thread/QueuedThreadPool.h"
#include "Resource/Asset/AssetManager.h"
#include "EditorSystem/EditorSystem.h"
#include "Platform/Input/InputSystem.h"

#include <array>
#include <cstdint>
#include <memory>

class EngineContext;

#define ENGINE_LOG_DEBUG(...) do { \
    SPDLOG_LOGGER_DEBUG(EngineContext::Log()->GetLogger(), __VA_ARGS__); \
} while (0)

#define ENGINE_LOG_INFO(...) do { \
    SPDLOG_LOGGER_INFO(EngineContext::Log()->GetLogger(), __VA_ARGS__); \
} while (0)

#define ENGINE_LOG_WARN(...) do { \
    SPDLOG_LOGGER_WARN(EngineContext::Log()->GetLogger(), __VA_ARGS__); \
} while (0)

#define ENGINE_LOG_FATAL(...) do { \
    SPDLOG_LOGGER_CRITICAL(EngineContext::Log()->GetLogger(), __VA_ARGS__); \
    throw std::runtime_error("");  \
} while (0)

#define ENGINE_TIME_SCOPE(name) TimeScopeHelper __timeScopeHelper(#name, EngineContext::GetTimeScope(EngineContext::CurrentFrameIndex()).get())
#define ENGINE_TIME_SCOPE_STR(name) TimeScopeHelper __timeScopeHelper(name, EngineContext::GetTimeScope(EngineContext::CurrentFrameIndex()).get())

class EngineContext
{
private:
    static std::shared_ptr<EngineContext> context;

public:
    EngineContext() = default;
    ~EngineContext() {};

    static std::shared_ptr<EngineContext> Init();
    static void MainLoop()                                          { context->MainLoopInternal(); };
    static void Destroy()                                           { context->DestroyInternal(); context = nullptr; }
    static bool Destroyed()                                         { return context == nullptr; }
    static uint32_t CurrentFrameIndex()                             { return context->currentFrameIndex; }
    static uint32_t PreviousFrameIndex()                            { return (context->currentFrameIndex + FRAMES_IN_FLIGHT - 1) % FRAMES_IN_FLIGHT; }
    static float GetDeltaTime()                                     { return context->deltaTime; }
    static std::shared_ptr<TimeScopes> GetTimeScope(uint32_t index) { return context->timers[index]; }

    static const std::shared_ptr<EngineContext>& Get() { return context; }

    static const std::shared_ptr<LogSystem>& Log()                         { return context->logSystem; }
    static const std::shared_ptr<QueuedThreadPool>& ThreadPool()           { return context->threadPool; }
    static const std::shared_ptr<RHIBackend>& RHI()                        { return context->rhiBackend; }
    static const std::shared_ptr<FileSystem>& File()                       { return context->fileSystem; }
    static const std::shared_ptr<AssetManager>& Asset()                    { return context->assetManager; }
    static const std::shared_ptr<WorldManager>& World()                    { return context->worldManager; }
    static const std::shared_ptr<RenderResourceManager>& RenderResource()  { return context->renderResourceManger; }
    static const std::shared_ptr<RenderSystem>& Render()                   { return context->renderSystem; }
    static const std::shared_ptr<EditorSystem>& Editor()                   { return context->editorSystem; }
    static const std::shared_ptr<InputSystem>& Input()                     { return context->inputSystem; }

private:
    std::shared_ptr<LogSystem> logSystem;
    std::shared_ptr<QueuedThreadPool> threadPool;
    std::shared_ptr<RHIBackend> rhiBackend;
    std::shared_ptr<FileSystem> fileSystem;
    std::shared_ptr<AssetManager> assetManager;
    std::shared_ptr<WorldManager> worldManager;
    std::shared_ptr<RenderResourceManager> renderResourceManger;
    std::shared_ptr<RenderSystem> renderSystem;
    std::shared_ptr<EditorSystem> editorSystem;
    std::shared_ptr<InputSystem> inputSystem;

    float deltaTime;
    uint32_t currentFrameIndex = 0;
    TimeScope timer;
    std::array<std::shared_ptr<TimeScopes>, FRAMES_IN_FLIGHT> timers;

    void DestroyInternal();
    void MainLoopInternal();
};