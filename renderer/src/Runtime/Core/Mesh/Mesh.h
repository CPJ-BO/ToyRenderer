#pragma once

#include "Core/Math/BoundingBox.h"
#include "Core/Math/Math.h"
#include "Core/Serialize/Serializable.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>


typedef struct BoneInfo
{
    std::string name;       //骨骼名称
    int index;              //骨骼索引
    Mat4 offset;            //骨骼的局部空间偏移矩阵

private:
    BeginSerailize()
    SerailizeEntry(name)
    SerailizeEntry(index)
    SerailizeEntry(offset)
    EndSerailize
    
} BoneInfo;

class Cluster;

class Mesh
{
public:
    Mesh() = default;
    Mesh(const Mesh& mesh, const std::vector<uint32_t>& subMeshIndex = {});
    ~Mesh() {};

    void Merge(const Mesh& other, const std::vector<uint32_t>& = {});

    inline bool HasPosition()   { return position.size(); }
    inline bool HasNormal()     { return normal.size(); }
    inline bool HasTangent()    { return tangent.size(); }
    inline bool HasTexCoord()   { return texCoord.size(); }
    inline bool HasColor()      { return color.size(); }
    inline bool HasBoneIndex()  { return boneIndex.size(); }
    inline bool HasBoneWeight() { return boneWeight.size(); }

    std::string name;
    AxisAlignedBox aabb;
    BoundingSphere sphere;
    BoundingBox box;

    std::vector<Vec3> position;
    std::vector<Vec3> normal;
    std::vector<Vec4> tangent;
    std::vector<Vec2> texCoord;
    std::vector<Vec3> color;
    std::vector<IVec4> boneIndex;
    std::vector<Vec4> boneWeight;

    std::vector<uint32_t> index;

    std::vector<BoneInfo> bone;

    // std::shared_ptr<VirtualMesh> virtualMesh;
    // std::vector<std::shared_ptr<Cluster>> clusters;

    inline uint32_t TriangleNum() { return index.size() / 3; }    //mesh所有的都是独立三角面

private:
    BeginSerailize()
    SerailizeEntry(name)
    SerailizeEntry(aabb)
    SerailizeEntry(sphere)
    SerailizeEntry(box)
    SerailizeEntry(position)
    SerailizeEntry(normal)
    SerailizeEntry(tangent)
    SerailizeEntry(texCoord)
    SerailizeEntry(color)
    SerailizeEntry(boneIndex)
    SerailizeEntry(boneWeight)
    SerailizeEntry(index)
    SerailizeEntry(bone)
    EndSerailize
};
typedef std::shared_ptr<Mesh> MeshRef;

