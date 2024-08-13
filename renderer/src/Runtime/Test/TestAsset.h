#pragma once

#include "Function/Global/EngineContext.h"
#include "Function/Render/RenderResource/Material.h"
#include "Function/Render/RenderResource/Model.h"
#include "Function/Render/RenderResource/Shader.h"

extern std::string shaderPath;

static void TestAsset()
{
    ModelProcessSetting processSetting = {
        .smoothNormal = false,           
        .flipUV = false,                       
        .loadMaterials = false,              
        .tangentSpace = true,                  
        .generateBVH = false,                  
        .generateCluster = false,            
        .generateVirtualMesh = false,           
    };
    std::shared_ptr<Model> model = std::make_shared<Model>("resource/build_in/model/Klee/klee.fbx", processSetting);
    EngineContext::Asset()->SaveAsset(model, "resource/build_in/config/model/klee.model");  
    // model = EngineContext::Asset()->GetOrLoadAsset<Model>("resource/build_in/config/model/klee.model");

    TextureRef texture = std::make_shared<Texture>("resource/build_in/model/Klee/Texture/dressing.jpg");
    EngineContext::Asset()->SaveAsset(texture, "resource/build_in/config/texture/dressing.texr");
    // texture = EngineContext::Asset()->GetOrLoadAsset<Texture>("resource/build_in/config/texture/dressing.texr");

    MaterialRef material = std::make_shared<Material>();
    material->textureDiffuse = texture;
    EngineContext::Asset()->SaveAsset(material, "resource/build_in/config/material/test.mtl");

    VertexBufferRef vertexBuffer = model->GetVertexBuffer(0);
    IndexBufferRef indexBuffer = model->GetIndexBuffer(0);

    std::shared_ptr<ArrayBuffer<uint32_t, 100>> arrayBuffer = std::make_shared<ArrayBuffer<uint32_t, 100>>();

    ShaderRef shader = std::make_shared<Shader>(shaderPath + "test/default.vert.spv", SHADER_FREQUENCY_VERTEX, "main");
}