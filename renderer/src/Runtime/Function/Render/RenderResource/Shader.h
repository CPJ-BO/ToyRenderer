#pragma once

#include "Core/Serialize/Serializable.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "Resource/Asset/Asset.h"

// UE的提供了着色器的预编译，缓存，ShaderMap，permutation管理，跨平台（HLSL,GLSL等）
// 此外，UE的着色器设计主要使用uber shader，并在cpp端对每个shader都有一个对应的cpp类做映射管理
class Shader : public Asset
{
public:
    Shader() {};
    Shader(const std::string& path, ShaderFrequency frequency, const std::string& entry = "main");

    virtual std::string GetAssetTypeName() override 			{ return "Shader Asset"; }
    virtual AssetType GetAssetType() override                   { return ASSET_TYPE_SHADER; }

    virtual void OnLoadAsset() override;

    inline std::string GetFilePath() { return path; }

    RHIShaderRef shader;

private:
    std::string path;
    ShaderFrequency frequency;
    std::string entry;

private:
    BeginSerailize()
    SerailizeEntry(path)
    SerailizeEntry(frequency)
    SerailizeEntry(entry)
    EndSerailize

    EnableAssetEditourUI()
};
typedef std::shared_ptr<Shader> ShaderRef;
