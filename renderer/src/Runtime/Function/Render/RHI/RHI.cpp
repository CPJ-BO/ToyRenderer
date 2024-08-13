
#include "RHI.h"
#include "RHIResource.h"
#include "RHIStructs.h"
#include "VulkanRHI/VulkanRHI.h"
#include "Core/Log/Log.h"

#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

std::shared_ptr<RHIBackend> RHIBackend::backend = nullptr;

std::shared_ptr<RHIBackend> RHIBackend::Init(const RHIBackendInfo& info)
{
    if(backend == nullptr)
    {
        switch (info.type) {
        case BACKEND_VULKAN:
            backend = std::make_shared<VulkanRHIBackend>(info); break;                 
        default:
            LOG_FATAL("Not implemented backend type!");
        }
    }

    return backend;
}

void RHIBackend::Tick()
{
    for(auto& resources : resourceMap)
    {
        for(RHIResourceRef& resource : resources)
        {
            if(resource)
            {
                if(resource.use_count() == 1)   resource->lastUseTick++;
                else                            resource->lastUseTick = 0;

                if(resource->lastUseTick > 6)  //析构资源6帧后销毁
                {
                    if(resource->GetType() != RHI_RENDER_PASS) 
                        std::cout << "RHI resource [" << resource.get() << "] of type [" << resource->GetType() << "] destroied" << std::endl;

                    resource->Destroy();
                    resource = nullptr;
                }
            }
        }
        resources.erase(std::remove_if(resources.begin(), resources.end(), [](RHIResourceRef x) {
            return x == nullptr;
        }), resources.end());
    }
}

void RHIBackend::Destroy()
{
    for(int32_t i = resourceMap.size() - 1; i >= 0; i--)   // 倒序析构
    {
        auto& resources = resourceMap[i];
        for(RHIResourceRef& resource : resources)
        {
            if(resource)
            {
                if(resource->GetType() != RHI_RENDER_PASS) 
                    std::cout << "RHI resource [" << resource.get() << "] of type [" << resource->GetType() << "] destroied" << std::endl;

                resource->Destroy();
            }
        }
    }
}