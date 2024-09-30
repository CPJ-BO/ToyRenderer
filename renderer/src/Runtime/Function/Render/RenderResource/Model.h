#pragma once

#include "Core/Mesh/Mesh.h"
#include "Core/Mesh/VirtualMesh/VirtualMesh.h"
#include "Core/Serialize/Serializable.h"
#include "Core/Util/IndexAlloctor.h"
#include "Function/Render/RenderResource/Buffer.h"
#include "Function/Render/RenderResource/Texture.h"
#include "Material.h"
#include "Resource/Asset/Asset.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct CachedSubmeshData
{
    std::vector<MeshClusterRef> clusters;                     
    std::shared_ptr<VirtualMesh> virtualMesh;    

private:
    BeginSerailize()
    SerailizeEntry(clusters)
    SerailizeEntry(virtualMesh)
    EndSerailize
};

class ModelCache : public Asset
{
public:
    std::vector<CachedSubmeshData> submeshes;

private:
    BeginSerailize()
    SerailizeBaseClass(Asset)
    SerailizeEntry(submeshes)
    EndSerailize              
};

typedef struct ModelProcessSetting
{
    bool smoothNormal = false;                  // 生成平滑法线
    bool flipUV = false;                        // 翻转UV
    bool loadMaterials = false;                 // 读取文件中的材质并生成材质资源
    bool tangentSpace = false;                  // 生成切线
    bool generateBVH = false;                   // 生成BVH
    bool generateCluster = false;               // 生成Cluster
    bool generateVirtualMesh = false;           // 生成虚拟几何体
    bool cacheCluster = false;                  // 对于虚拟几何体和Cluster做缓存，只需要生成一次

private:
    BeginSerailize()
    SerailizeEntry(smoothNormal)
    SerailizeEntry(flipUV)
    SerailizeEntry(loadMaterials)
    SerailizeEntry(tangentSpace)
    SerailizeEntry(generateBVH)
    SerailizeEntry(generateCluster)
    SerailizeEntry(generateVirtualMesh)
    SerailizeEntry(cacheCluster)
    EndSerailize

}ModelProcessSetting;

struct SubmeshData
{
    std::shared_ptr<Mesh> mesh;                                 // CPU端的mesh和cluster信息
    std::vector<MeshClusterRef> clusters;                       // 仅生成cluster时的信息
    std::shared_ptr<VirtualMesh> virtualMesh;                   // 生成cluster + cluster group时的信息

    VertexBufferRef vertexBuffer;                               // GPU端的顶点和索引缓冲，既可能存储单个submesh的全部顶点和索引，也可能存储其全部cluster合并后的数据
    IndexBufferRef indexBuffer;

    IndexRange meshClusterID = { 0, 0 };            // 提交的一组cluster的ID范围
    IndexRange meshClusterGroupID = { 0, 0 };       // 提交的一组cluster group的ID范围
};

// 目前Model并不提供从程序写入模型文件的功能
class Model : public Asset, public AssetBinder
{
public:
    Model(std::string path, ModelProcessSetting processSetting);
    ~Model();

    virtual std::string GetAssetTypeName() override 		    { return "Model Asset"; }
    virtual AssetType GetAssetType() override                   { return ASSET_TYPE_MODEL; }

    virtual void OnLoadAsset() override;
    virtual void OnSaveAsset() override;

    // inline std::string GetPath()                                { return path; }
    // inline ModelProcessSetting GetProcessSetting()              { return processSetting; }

    // void SetPath(std::string path)                              { this->path = path; }
    // void SetProcessSetting(ModelProcessSetting processSetting)  { this->processSetting = processSetting; }

    inline uint32_t GetSubmeshCount()                           { return submeshes.size(); }
    VertexBufferRef GetVertexBuffer(uint32_t subMeshIndex)      { return submeshes[subMeshIndex].vertexBuffer; }
    IndexBufferRef GetIndexBuffer(uint32_t subMeshIndex)        { return submeshes[subMeshIndex].indexBuffer; }
    MaterialRef GetMaterial(uint32_t subMeshIndex)              { return materials[subMeshIndex]; }

    const SubmeshData& Submesh(uint32_t subMeshIndex)           { return submeshes[subMeshIndex]; }

protected:
    bool LoadFromFile(std::string path);
    void ProcessNode(aiNode* node, const aiScene* scene, std::vector<aiMesh*>& processMeshes);
    void ProcessMesh(aiMesh* mesh, const aiScene* scene, int index);
    void ExtractBoneWeights(Mesh* submesh, aiMesh* mesh, const aiScene* scene);
    std::shared_ptr<Texture> LoadMaterialTexture(aiMaterial* mat, aiTextureType type);

    std::vector<SubmeshData> submeshes;
    std::vector<MaterialRef> materials;
    std::shared_ptr<ModelCache> cache;

    std::string path;
    ModelProcessSetting processSetting;

    uint64_t totalIndex = 0;    // 统计信息
    uint64_t totalVertex = 0;
    uint32_t totalClusterCnt = 0;
    uint32_t totalClusterMaxMip = 0;

    std::unordered_map<std::string, TextureRef> textureMap; // 加载材质时，单物体可能有多个重复的纹理引用，做个cache避免重复加载

private:
    Model() = default;
    
    BeginSerailize()
    SerailizeBaseClass(Asset)
    SerailizeBaseClass(AssetBinder)
    SerailizeEntry(path)
    SerailizeEntry(processSetting)
    EndSerailize

    EnableAssetEditourUI()
};
typedef std::shared_ptr<Model> ModelRef;


