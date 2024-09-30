#pragma once

#include "Core/Math/Math.h"
#include "Core/Serialize/Serializable.h"
#include "Function/Render/RHI/RHIStructs.h"
#include "Function/Render/RenderResource/RenderStructs.h"
#include "Function/Render/RenderResource/Shader.h"
#include "Function/Render/RenderResource/Texture.h"
#include "Resource/Asset/Asset.h"
#include <array>
#include <cstdint>
#include <memory>

enum RenderPassMaskBits
{
    PASS_MASK_NONE = 0x00000000,
    // PASS_MASK_DEPTH_PASS = 0x00000001,           // 暂时不支持，深度pass都使用默认着色器，后面再支持override吧
    PASS_MASK_FORWARD_PASS = 0x00000001,
    PASS_MASK_DEFERRED_PASS = 0x00000002,
    PASS_MASK_TRANSPARENT_PASS = 0x00000004,
    // PASS_MASK_POST_PROCESS_PASS = 0x00000008,    // 后处理的screen pass也暂时不支持

	PASS_MASK_MAX_ENUM = 0x7FFFFFFF,	//
};
typedef uint32_t RenderPassMasks;

// 约等于shaderlab文件的抽象，拆成了Material+索引各个shader文件
// 到底应该如何抽象材质类？不是不能参考，但是太复杂了
// 目前选择不提供完整的光栅管线状态设置，仅设置材质的参数信息和着色器信息，（以及对应的延迟/前向管线？）
// TODO 给材质提供Uniform buffer？也只需要bindless就可以实现
class Material : public Asset, public AssetBinder
{
public:
    Material();
    Material(const Material& other) = default;  // 可以拷贝构造
    ~Material();

    virtual std::string GetAssetTypeName() override 		{ return "Material Asset"; }
    virtual AssetType GetAssetType() override               { return ASSET_TYPE_MATERIAL; }

    virtual void OnLoadAsset() override;
    virtual void OnSaveAsset() override;

    uint32_t GetMaterialID()                                { return materialID; }

    // 材质参数
    void SetDiffuse(Vec4 diffuse)                           { this->diffuse = diffuse;      Update(); }
    void SetEmission(Vec4 emission)                         { this->emission = emission;    Update(); }
    void SetRoughness(float roughness)                      { this->roughness = roughness;  Update(); }
    void SetMetallic(float metallic)                        { this->metallic = metallic;    Update(); }
    void SetAlphaClip(float alphaClip)                      { this->alphaClip = alphaClip;  Update(); }
    void SetInt(int32_t data, uint32_t index)               { ints[index] = data;           Update(); }
    void SetFloat(float data, uint32_t index)               { floats[index] = data;         Update(); }
    void SetColor(Vec4 data, uint32_t index)                { colors[index] = data;         Update(); }
    void SetDiffuse(TextureRef texture)                     { textureDiffuse = texture;     Update(); }
    void SetNormal(TextureRef texture)                      { textureNormal = texture;      Update(); }
    void SetARM(TextureRef texture)                         { textureArm = texture;         Update(); }
    void SetSpecular(TextureRef texture)                    { textureSpecular = texture;    Update(); }
    void SetTexture2D(TextureRef texture, uint32_t index)   { texture2D[index] = texture;   Update(); } 
    void SetTextureCube(TextureRef texture, uint32_t index) { textureCube[index] = texture; Update(); } 
    void SetTexture3D(TextureRef texture, uint32_t index)   { texture3D[index] = texture;   Update(); } 
    void SetVertexShader(ShaderRef shader)                  { vertexShader = shader; }
    void SetGeometryShader(ShaderRef shader)                { geometryShader = shader; }
    void SetFragmentShader(ShaderRef shader)                { fragmentShader = shader; }
    ShaderRef GetVertexShader()                             { return vertexShader; }
    ShaderRef GetGeometryShader()                           { return geometryShader; }
    ShaderRef GetFragmentShader()                           { return fragmentShader; }

    // 渲染管线设置
    uint32_t RenderQueue()                                  { return renderQueue; }
    RenderPassMasks RenderPassMask()                        { return renderPassMask; }  
    RasterizerCullMode CullMode()                           { return cullMode; }  
    RasterizerFillMode GetFillMode()                        { return fillMode; }  
    bool DepthTest()                                        { return depthTest;}
    bool DepthWrite()                                       { return depthWrite; }
    CompareFunction DepthCompare()                          { return depthCompare; }
    bool UseForDepthPass()                                  { return useForDepthPass; }
    bool CastShadow()                                       { return castShadow; }

    void SetRenderQueue(uint32_t queue)                     { renderQueue = queue; }
    void SetRenderPassMask(RenderPassMasks mask)            { renderPassMask = mask; }
    void SetCullMode(RasterizerCullMode cull)               { cullMode = cull; }
    void SetFillMode(RasterizerFillMode fill)               { fillMode = fill; }
    void SetDepthTest(bool test)                            { depthTest = test;}
    void SetDepthWrite(bool write)                          { depthWrite = write; }
    void SetDepthCompare(CompareFunction compare)           { depthCompare = compare; }
    void SetUseForDepthPass(bool use)                       { useForDepthPass = use; }
    void SetCastShadow(bool shadow)                         { castShadow = shadow; }

protected:
    Vec4 diffuse = Vec4::Ones();
    Vec4 emission = Vec4::Zero();

    float roughness = 0.5f;
    float metallic = 0.0f;
    float alphaClip = 0.0f;

    std::array<int32_t, 8> ints = { 0 };
    std::array<float, 8> floats = { 0.0f };
    std::array<Vec4, 8> colors = { Vec4::Zero() };

    TextureRef textureDiffuse;
    TextureRef textureNormal;
    TextureRef textureArm;
    TextureRef textureSpecular;

    std::array<TextureRef, 8> texture2D;
    std::array<TextureRef, 4> textureCube;
    std::array<TextureRef, 4> texture3D;

    ShaderRef vertexShader;                                     // 材质使用的着色器，若为空则可能使用各个pass的默认着色器
    ShaderRef geometryShader;
    ShaderRef fragmentShader;

    void Update();

protected:
    uint32_t renderQueue = 1000;                                // 用于指示渲染顺序
    RenderPassMasks renderPassMask = PASS_MASK_DEFERRED_PASS;   // 用于指示和标记特定pass，方便对应的mesh pass收集

    RasterizerCullMode cullMode = CULL_MODE_BACK;               // 剔除模式
    RasterizerFillMode fillMode = FILL_MODE_SOLID;              // 填充模式
    bool depthTest = true;                                      // 深度测试
    bool depthWrite = true;                                     // 深度写入
    CompareFunction depthCompare = COMPARE_FUNCTION_LESS_EQUAL; // 深度测试函数
                                                                // TODO 是否加入混合信息？

    bool useForDepthPass = true;                                // 是否加入深度pass渲染
    bool castShadow = true;                                     // 是否加入阴影pass渲染
    

    MaterialInfo materialInfo;      // 提交给GPU的信息和下标ID
    uint32_t materialID;

private:
    BeginSerailize()
    SerailizeBaseClass(Asset)
    SerailizeBaseClass(AssetBinder)
    SerailizeEntry(diffuse)
    SerailizeEntry(emission)
    SerailizeEntry(roughness)
    SerailizeEntry(metallic)
    SerailizeEntry(alphaClip)
    SerailizeEntry(ints)
    SerailizeEntry(floats)
    SerailizeEntry(colors)
    SerailizeEntry(renderQueue)
    SerailizeEntry(renderPassMask)
    SerailizeEntry(cullMode)
    SerailizeEntry(fillMode)
    SerailizeEntry(depthTest)
    SerailizeEntry(depthWrite)
    SerailizeEntry(depthCompare)
    SerailizeEntry(castShadow)
    EndSerailize

    EnableAssetEditourUI()
};
typedef std::shared_ptr<Material> MaterialRef;



