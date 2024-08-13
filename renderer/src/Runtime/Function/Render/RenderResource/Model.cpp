

#include "Function/Render/RenderResource/Model.h"
#include "Core/Log/log.h"
#include "Core/Mesh/TangentSpace.h"
#include "Function/Global/EngineContext.h"
#include <cstdint>

CEREAL_REGISTER_TYPE(Model)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Asset, Model)

void Model::OnLoadAsset()
{
    LoadFromFile(path);

    for(uint32_t i = 0; i < meshes.size(); i++)
    {
        VertexBufferRef vertexBuffer = std::make_shared<VertexBuffer>();
        vertexBuffer->SetPosition(meshes[i]->position);
        vertexBuffer->SetNormal(meshes[i]->normal);
        vertexBuffer->SetTangent(meshes[i]->tangent);
        vertexBuffer->SetTexCoord(meshes[i]->texCoord);
        vertexBuffer->SetColor(meshes[i]->color);
        vertexBuffer->SetBoneIndex(meshes[i]->boneIndex);
        vertexBuffer->SetBoneWeight(meshes[i]->boneWeight);
        vertexBuffers.push_back(vertexBuffer);

        IndexBufferRef indexBuffer = std::make_shared<IndexBuffer>();
        indexBuffer->SetIndex(meshes[i]->index);
        indexBuffers.push_back(indexBuffer);
    }
}

Model::Model(std::string path, ModelProcessSetting processSetting)
: path(path)
, processSetting(processSetting)
{
    OnLoadAsset();
}

bool Model::LoadFromFile(std::string path)
{
    if (processSetting.generateVirtualMesh) processSetting.smoothNormal = true;  //对于生成虚拟几何体需要顶点去重，强制平滑法线

    uint32_t processSteps = aiProcess_Triangulate | aiProcess_FixInfacingNormals;
    if (processSetting.flipUV) processSteps |= aiProcess_FlipUVs;
    if (processSetting.smoothNormal) processSteps |= aiProcess_DropNormals | aiProcess_GenSmoothNormals;
    if (!processSetting.smoothNormal) processSteps |= aiProcess_JoinIdenticalVertices | aiProcess_GenNormals;  //不需要平滑法线就可以合并重复顶点了，

    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(EngineContext::File()->Absolute(path), processSteps);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        LOG_FATAL("Assimp load error : %s", import.GetErrorString());
        return false;
    }

    std::vector<aiMesh*> processMeshes;
    ProcessNode(scene->mRootNode, scene, processMeshes);
    meshes.resize(processMeshes.size());

    //并行加载各个子mesh
    for(int i = 0; i < processMeshes.size(); i++)
    {
        aiMesh* mesh = processMeshes[i];
        LOG_DEBUG("[%d/%d] Start processing mesh [%s].", i, scene->mNumMeshes, mesh->mName.C_Str());

        ProcessMesh(mesh, scene, i);
    }  

    return true;
}

void Model::ProcessNode(aiNode* node, const aiScene* scene, std::vector<aiMesh*>& processMeshes)
{
    // 处理节点所有的网格（如果有的话）
    for (uint32_t i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        processMeshes.push_back(mesh);
    }
    // 接下来对它的子节点重复这一过程
    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, processMeshes);
    }
}

void Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, int index)
{
    std::shared_ptr<Mesh> submesh = std::make_shared<Mesh>();

    // 处理顶点位置
    submesh->position = std::vector<Vec3>(mesh->mNumVertices);
    for (uint32_t i = 0; i < mesh->mNumVertices; i++)
    {         
        submesh->position[i](0) = mesh->mVertices[i].x;
        submesh->position[i](1) = mesh->mVertices[i].y;
        submesh->position[i](2) = mesh->mVertices[i].z;       
    }

    // 处理顶点法线
    if (mesh->mNormals) 
    {
        submesh->normal = std::vector<Vec3>(mesh->mNumVertices);
        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {  
            submesh->normal[i](0) = mesh->mNormals[i].x;
            submesh->normal[i](1) = mesh->mNormals[i].y;
            submesh->normal[i](2) = mesh->mNormals[i].z;
        }
    }

    // 处理顶点切线
    if (mesh->mTangents)
    {
        submesh->tangent = std::vector<Vec4>(mesh->mNumVertices);
        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {  
            submesh->tangent[i](0) = mesh->mTangents[i].x;
            submesh->tangent[i](1) = mesh->mTangents[i].y;
            submesh->tangent[i](2) = mesh->mTangents[i].z;
            submesh->tangent[i](3) = 1.0f;  //最后一位为符号(手性)
        }
    }
    else if (processSetting.tangentSpace)
    {
        submesh->tangent = std::vector<Vec4>(mesh->mNumVertices);

        TangentSpace tangentCalculator = TangentSpace();
        tangentCalculator.Generate(submesh.get());
    }

    // 处理顶点颜色
    if (mesh->mColors[0])
    {
        submesh->color = std::vector<Vec3>(mesh->mNumVertices);
        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {  
            submesh->color[i](0) = mesh->mColors[0][i].r;
            submesh->color[i](1) = mesh->mColors[0][i].g;
            submesh->color[i](2) = mesh->mColors[0][i].b;
        }
    }
        
    // 处理顶点纹理坐标
    if (mesh->mTextureCoords[0])
    {
        submesh->texCoord = std::vector<Vec2>(mesh->mNumVertices);
        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {  
            submesh->texCoord[i](0) = mesh->mTextureCoords[0][i].x;
            submesh->texCoord[i](1) = mesh->mTextureCoords[0][i].y;   
        }
    }
    // if (mesh->mTextureCoords[1])
    // {
    //     submesh->texCoord = std::vector<Vec2>(mesh->mNumVertices);
    //     for (uint32_t i = 0; i < mesh->mNumVertices; i++)
    //     {  
    //         submesh->texCoord[i](0) = mesh->mTextureCoords[1][i].x;
    //         submesh->texCoord[i](1) = mesh->mTextureCoords[1][i].y;    
    //     }
    // }

    // 处理骨骼
    if (mesh->HasBones())   ExtractBoneWeights(submesh.get(), mesh, scene);

  
    // 处理索引,已经三角面化了就全当3处理了
    submesh->index = std::vector<uint32_t>(mesh->mNumFaces * 3);

    int tempCnt = 0;
    for (uint32_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; j++)
        {
            submesh->index[j + tempCnt] = face.mIndices[j];
        }
        tempCnt += face.mNumIndices;
    }

    // 处理包围盒
    submesh->aabb = AxisAlignedBox(submesh->position[0], Vec3::Zero());
    for (uint32_t i = 0; i < mesh->mNumVertices; i++)   submesh->aabb.Merge(submesh->position[i]);

    // 处理mesh名称
    submesh->name = std::string(mesh->mName.C_Str());

    // 添加到mesh asset
    meshes[index] = submesh;


}

void Model::ExtractBoneWeights(Mesh* submesh, aiMesh* mesh, const aiScene* scene)
{
    submesh->boneIndex = std::vector<IVec4>(mesh->mNumVertices);
    submesh->boneWeight = std::vector<Vec4>(mesh->mNumVertices);

    // 将骨骼相关信息初始化
    for(int i = 0; i < mesh->mNumVertices; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            submesh->boneIndex[i](j) = -1;
            submesh->boneWeight[i](j) = 0.0f;
        }
    }

    // 遍历骨骼
    for (uint32_t index = 0; index < mesh->mNumBones; ++index)
    {
        int boneIndex = -1;
        std::string boneName = mesh->mBones[index]->mName.C_Str();

        bool find = false;
        for(int i = 0; i < submesh->bone.size(); i++)
        {
            if(submesh->bone[i].name.compare(boneName) == 0)
            {
                boneIndex = submesh->bone[i].index;
                find = true;
                break;
            }
        }
        if (!find)
        {
            BoneInfo newBoneInfo;
            newBoneInfo.index = (int)submesh->bone.size();
            newBoneInfo.name = boneName;
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    newBoneInfo.offset(i, j) = mesh->mBones[index]->mOffsetMatrix[i][j];
                }
            }
            newBoneInfo.offset.transposeInPlace();  // 要做一个转置？
            newBoneInfo.name = std::string(boneName);
            submesh->bone.push_back(newBoneInfo);

            boneIndex = newBoneInfo.index;
        }

        auto weights = mesh->mBones[index]->mWeights;
        int numWeights = mesh->mBones[index]->mNumWeights;

        // 处理和该骨骼相关的顶点
        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
        {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;

            for (int i = 0; i < 4; ++i)
            {
                if(submesh->boneIndex[vertexId](i) < 0)
                {
                    submesh->boneIndex[vertexId](i) = boneIndex;
                    submesh->boneWeight[vertexId](i) = weight;
                    break;
                }
            }
        }
    }
}