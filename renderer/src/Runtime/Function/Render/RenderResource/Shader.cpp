#include "Shader.h"
#include "Function/Global/EngineContext.h"

Shader::Shader(const std::string& path, ShaderFrequency frequency, const std::string& entry)
: path(path)
{
    shader = EngineContext::RenderResource()->GetOrCreateRHIShader(path, frequency, entry);
}