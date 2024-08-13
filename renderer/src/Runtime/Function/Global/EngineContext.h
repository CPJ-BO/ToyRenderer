#pragma once

#include "Core/Log/LogSystem.h"
#include "Core/Util/TimeScope.h"
#include "Function/Framework/World/WorldManager.h"
#include "Function/Render/RenderResource/RenderResourceManager.h"
#include "Platform/File/FileSystem.h"
#include "Function/Render/RHI/RHI.h"
#include "Platform/Thread/QueuedThreadPool.h"
#include "Resource/Asset/AssetManager.h"
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

class EngineContext
{
private:
    static std::shared_ptr<EngineContext> context;

public:
    EngineContext() = default;
    ~EngineContext() {};

    static std::shared_ptr<EngineContext> Init();
    static void Tick()                      { context->TickInternal(); }
    static void Destroy()                   { context->DestroyInternal(); context = nullptr; }
    static bool Destroyed()                 { return context == nullptr; }
    static uint32_t CurrentFrameIndex()     { return context->currentFrameIndex; }
    static float GetDeltaTime()             { return context->deltaTime; }

    static std::shared_ptr<EngineContext> Get() { return context; }

    static std::shared_ptr<LogSystem> Log()                         { return context->logSystem; }
    static std::shared_ptr<QueuedThreadPool> ThreadPool()           { return context->threadPool; }
    static std::shared_ptr<RHIBackend> RHI()                        { return context->rhiBackend; }
    static std::shared_ptr<FileSystem> File()                       { return context->fileSystem; }
    static std::shared_ptr<AssetManager> Asset()                    { return context->assetManager; }
    static std::shared_ptr<WorldManager> World()                    { return context->worldManager; }
    static std::shared_ptr<RenderResourceManager> RenderResource()  { return context->renderResourceManger; }

private:
    std::shared_ptr<LogSystem> logSystem;
    std::shared_ptr<QueuedThreadPool> threadPool;
    std::shared_ptr<RHIBackend> rhiBackend;
    std::shared_ptr<FileSystem> fileSystem;
    std::shared_ptr<AssetManager> assetManager;
    std::shared_ptr<WorldManager> worldManager;
    std::shared_ptr<RenderResourceManager> renderResourceManger;

    float deltaTime;
    uint32_t currentFrameIndex = 0;
    TimeScope timer;

    void TickInternal();

    void DestroyInternal();
};