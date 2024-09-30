#include "Mesh.h"
#include "Core/Math/BoundingBox.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <map>

Mesh::Mesh(const Mesh& mesh, const std::vector<uint32_t>& subMeshIndex)
{
    Merge(mesh, subMeshIndex);
}

void Mesh::Merge(const Mesh& other, const std::vector<uint32_t>& subMeshIndex)
{
    uint32_t baseIndexSize = index.size();  //如果该mesh已有顶点，那需要保证可用的流是一致的
    if(baseIndexSize > 0)    
    {
        assert((position.size() > 0 && other.position.size() > 0)   || (position.size() == 0 && other.position.size() == 0));
        assert((normal.size() > 0 && other.normal.size() > 0)       || (normal.size() == 0 && other.normal.size() == 0));
        assert((tangent.size() > 0 && other.tangent.size() > 0)     || (tangent.size() == 0 && other.tangent.size() == 0));
        assert((texCoord.size() > 0 && other.texCoord.size() > 0)   || (texCoord.size() == 0 && other.texCoord.size() == 0));
        assert((color.size() > 0 && other.color.size() > 0)         || (color.size() == 0 && other.color.size() == 0));

        // 不合并骨骼
        assert(boneIndex.size() == 0 && other.boneIndex.size() == 0);
        assert(boneWeight.size() == 0 && other.boneWeight.size() == 0);
    }
    
    std::map<uint32_t, uint32_t> indexMap;
    uint32_t baseIndex = position.size();
    auto& subIndex = subMeshIndex.size() > 0 ? subMeshIndex : other.index;
    for(auto& index : subIndex)
    {
        if(indexMap.find(index) == indexMap.end())  indexMap[index] = baseIndex++;
        this->index.push_back(indexMap[index]);
    }
    if(other.position.size() > 0) 
    {
        this->position.resize(this->position.size() + indexMap.size());
        for(auto& pair : indexMap)  this->position[pair.second] = other.position[pair.first];
    }
    if(other.normal.size() > 0) 
    {
        this->normal.resize(this->normal.size() + indexMap.size());
        for(auto& pair : indexMap)  this->normal[pair.second] = other.normal[pair.first];
    }
    if(other.tangent.size() > 0) 
    {
        this->tangent.resize(this->tangent.size() + indexMap.size());
        for(auto& pair : indexMap)  this->tangent[pair.second] = other.tangent[pair.first];
    }
    if(other.texCoord.size() > 0) 
    {
        this->texCoord.resize(this->texCoord.size() + indexMap.size());
        for(auto& pair : indexMap)  this->texCoord[pair.second] = other.texCoord[pair.first];
    }
    if(other.color.size() > 0) 
    {
        this->color.resize(this->color.size() + indexMap.size());
        for(auto& pair : indexMap)  this->color[pair.second] = other.color[pair.first];
    }

    // 包围盒
    if(baseIndexSize == 0) this->aabb = AxisAlignedBox(this->position[0], {0, 0,0});    
    this->aabb.Merge(other.aabb);
    this->sphere = BoundingSphere(this->aabb);
    this->box = BoundingBox(this->aabb);
}
