#pragma once

#include "Function/Render/RHI/RHIStructs.h"

// UE的提供了着色器的预编译，缓存，ShaderMap，permutation管理，跨平台（HLSL,GLSL等）
// 此外，UE的着色器设计主要使用uber shader，并在cpp端对每个shader都有一个对应的cpp类做映射管理
class Shader
{
public:
    Shader(const std::string& path, ShaderFrequency frequency, const std::string& entry = "main");

    RHIShaderRef shader;

    inline std::string GetFilePath() { return path; }

private:
    std::string path;
};
typedef std::shared_ptr<Shader> ShaderRef;
