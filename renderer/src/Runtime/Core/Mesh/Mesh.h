#pragma once

#include "Core/Math/BoundingBox.h"
#include "Core/Math/Math.h"

#include <string>
#include <vector>


typedef struct BoneInfo
{
    std::string name;       //骨骼名称
    int index;              //骨骼索引
    Mat4 offset;            //骨骼的局部空间偏移矩阵
} BoneInfo;

class Cluster;

class Mesh
{
public:
    Mesh() = default;
    ~Mesh() {};

    std::string name;
    AxisAlignedBox aabb;

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
};
