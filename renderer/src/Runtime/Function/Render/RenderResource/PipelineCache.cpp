#include "PipelineCache.h"
#include "Function/Global/EngineContext.h"
#include "Function/Render/RHI/RHIStructs.h"

#include "Core/Log/Log.h"

GraphicsPipelineCache::CachedPipeline GraphicsPipelineCache::Allocate(const RHIGraphicsPipelineInfo& info)
{
    GraphicsPipelineCache::CachedPipeline ret;

    auto iter = cachedPipelines.find(info);
    if(iter != cachedPipelines.end())
    return iter->second;
    
    if(!IsValid(info))
    {
        // LOG_DEBUG("RHIGraphicsPipelineInfo is not valid!");
        return { nullptr };
    }

    LOG_DEBUG("RHIGraphicsPipeline not found in cache, creating new.");
    ret = {
        .pipeline = EngineContext::RHI()->CreateGraphicsPipeline(info)
    };
    cachedPipelines[info] = ret;
    
    return ret;
}

bool GraphicsPipelineCache::IsValid(RHIGraphicsPipelineInfo info)
{
    if(!info.vertexShader || !info.fragmentShader || !info.rootSignature) return false;

    for(auto& input : info.vertexShader->GetReflectInfo().inputVariables) 
    {
        if(input != FORMAT_UKNOWN) return false;    // 目前使用的管线里全部bindless，所以顶点输入一定为空
    }
    if(info.geometryShader)
    {
        if(info.vertexShader->GetReflectInfo().outputVariables != info.geometryShader->GetReflectInfo().inputVariables) return false;
        if(info.geometryShader->GetReflectInfo().outputVariables != info.fragmentShader->GetReflectInfo().inputVariables) return false;
    }
    else
    {
        if(info.vertexShader->GetReflectInfo().outputVariables != info.fragmentShader->GetReflectInfo().inputVariables) return false;
    }
    for(uint32_t i = 0; i < std::min(info.colorAttachmentFormats.size(), info.fragmentShader->GetReflectInfo().outputVariables.size()); i++)
    {
        if( FormatChanelCounts(info.colorAttachmentFormats[i]) != 
            FormatChanelCounts(info.fragmentShader->GetReflectInfo().outputVariables[i])) return false;    // 输出通道数一致
    }

    // TODO 描述符等信息的检测
    return true;
} 