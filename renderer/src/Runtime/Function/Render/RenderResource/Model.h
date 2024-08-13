#pragma once

#include "Core/Mesh/Mesh.h"
#include "Function/Render/RenderResource/Buffer.h"
#include "Resource/Asset/Asset.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <cstdint>
#include <memory>
#include <string>

typedef struct ModelProcessSetting
{
    bool smoothNormal = false;                  // 生成平滑法线
    bool flipUV = false;                        // 翻转UV
    bool loadMaterials = false;                 // 读取文件中的材质并生成材质资源
    bool tangentSpace = false;                  // 生成切线
    bool generateBVH = false;                   // 生成BVH
    bool generateCluster = false;               // 生成Cluster
    bool generateVirtualMesh = false;           // 生成虚拟几何体

private:
    BeginSerailize()
    SerailizeEntry(smoothNormal)
    SerailizeEntry(flipUV)
    SerailizeEntry(loadMaterials)
    SerailizeEntry(tangentSpace)
    SerailizeEntry(generateBVH)
    SerailizeEntry(generateCluster)
    SerailizeEntry(generateVirtualMesh)
    EndSerailize

}ModelProcessSetting;

// 目前Model并不提供从程序写入模型文件的功能
class Model : public Asset
{
public:
    Model(std::string path, ModelProcessSetting processSetting);

    virtual AssetType GetType() override                        { return ASSET_TYPE_MODEL; }

    // inline std::string GetPath()                                { return path; }
    // inline ModelProcessSetting GetProcessSetting()              { return processSetting; }

    // void SetPath(std::string path)                              { this->path = path; }
    // void SetProcessSetting(ModelProcessSetting processSetting)  { this->processSetting = processSetting; }

    inline uint32_t GetSubMeshNum()                             { return meshes.size(); }
    VertexBufferRef GetVertexBuffer(uint32_t subMeshIndex)      { return vertexBuffers[subMeshIndex]; }
    IndexBufferRef GetIndexBuffer(uint32_t subMeshIndex)        { return indexBuffers[subMeshIndex]; }

protected:
    virtual void OnLoadAsset() override;

    bool LoadFromFile(std::string path);
    void ProcessNode(aiNode* node, const aiScene* scene, std::vector<aiMesh*>& processMeshes);
    void ProcessMesh(aiMesh* mesh, const aiScene* scene, int index);
    void ExtractBoneWeights(Mesh* submesh, aiMesh* mesh, const aiScene* scene);

    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<VertexBufferRef> vertexBuffers;
    std::vector<IndexBufferRef> indexBuffers;

    std::string path;
    ModelProcessSetting processSetting;

private:
    Model() = default;
    
    BeginSerailize()
    SerailizeBaseClass(Asset)
    SerailizeEntry(path)
    SerailizeEntry(processSetting)
    EndSerailize
};
typedef std::shared_ptr<Model> ModelRef;


