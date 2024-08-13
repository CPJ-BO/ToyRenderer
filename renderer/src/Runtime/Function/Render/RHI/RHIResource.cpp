#include "RHIResource.h"
#include "RHI.h"
#include "Core/Log/Log.h"
#include "Platform/HAL/ScopeLock.h"

#include <memory>

RHICommandListRef RHICommandPool::CreateCommandList(bool byPass)
{
    RHICommandContextRef context = nullptr;
    {
        ScopeLock lock(sync);
        if(idleContexts.size() > 0) 
        {
            context = idleContexts.front();
            idleContexts.pop();
        }
        else {
            context = RHIBackend::Get()->CreateCommandContext(this);
            contexts.push_back(context);
        }
    }
    
    CommandListInfo info = {
        .pool = this,
        .context = context,
        .byPass = byPass,
    };
    RHICommandListRef commandList = std::make_shared<RHICommandList>(info);
    return commandList;
}

Extent3D RHITexture::MipExtent(uint32_t mipLevel)
{
    if(mipLevel > info.mipLevels) LOG_DEBUG("Mip level is greater than texture`s max mip!");

    Extent3D extent = info.extent;
    extent.width = std::max((uint32_t)1, extent.width >> mipLevel);
    extent.height = std::max((uint32_t)1, extent.height >> mipLevel);
    extent.depth = std::max((uint32_t)1, extent.depth >> mipLevel);

    return extent;
}