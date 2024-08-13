#pragma once

#include "Function/Global/EngineContext.h"

static void TestSystem()
{
    EngineContext::Log()->Debug("Debug");
    EngineContext::Log()->Info("Info");
    EngineContext::Log()->Warn("Warn");
    ENGINE_LOG_DEBUG("Debug");
    ENGINE_LOG_INFO("Info"); 
    ENGINE_LOG_WARN("Warn");
    // ENGINE_LOG_FATAL("Fatal");

    // EngineContext::Log()->Fatal("Fatal");

    std::shared_ptr<FileSystem> fileSystem = EngineContext::File();
    std::vector<std::string> files = fileSystem->Traverse(".", true);
    for(auto& file : files)
    {
        std::cout   << file << " " 
                    << fileSystem->Filename(file) << " " 
                    << fileSystem->Extension(file) << " " 
                    << fileSystem->Extension(fileSystem->Filename(file)) << std::endl;
    }
}