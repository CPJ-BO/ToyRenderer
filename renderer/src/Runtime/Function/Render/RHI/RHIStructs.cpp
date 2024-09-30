#include "RHIStructs.h"
#include "RHIResource.h"
#include "Core/Log/Log.h"

#include <algorithm>

RHIRootSignatureInfo& RHIRootSignatureInfo::AddEntry(const ShaderResourceEntry& entry) 
{ 
    // 合并已有的 
    for (ShaderResourceEntry& oldEntry : entries)
    {
        if( oldEntry.set == entry.set && 
            oldEntry.binding == entry.binding)
        {
            if (oldEntry.type != entry.type) 
            {
                LOG_FATAL("Conflict shader resource entry!");
                return *this;
            }
            else 
            {
                if((oldEntry.size == 0 || entry.size == 0) && (oldEntry.size == 1 || entry.size == 1))  oldEntry.size = 0;
                else oldEntry.size = std::max(oldEntry.size, entry.size);        //反射得到的bindless数组的数量是0，把0当最大值吧
                                    
                oldEntry.frequency |= entry.frequency;
                return *this;
            }
        }
    }

    // 新加入
    entries.push_back(entry); 
    return *this; 
}

RHIRootSignatureInfo& RHIRootSignatureInfo::AddEntry(const RHIRootSignatureInfo& other)
{
    for(auto& entry : other.entries) AddEntry(entry);
    return *this;
}

RHIRootSignatureInfo& RHIRootSignatureInfo::AddEntryFromReflect(RHIShaderRef shader) 
{   
    ShaderReflectInfo reflectInfo = shader->GetReflectInfo();

    for(ShaderResourceEntry& reflectEntry : reflectInfo.resources) AddEntry(reflectEntry);
    return *this; 
}
