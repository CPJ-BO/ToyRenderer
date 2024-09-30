#pragma once

#include "Core/Mesh/Mesh.h"
#include "Core/Serialize/Serializable.h"
#include "Function/Global/Definations.h"

#include <memory>
#include <vector>



class MeshCluster 
{
public:
    static const uint32_t CLUSTER_SIZE = CLUSTER_TRIANGLE_SIZE;   //Cluster内最大三角形数目

    std::shared_ptr<Mesh> mesh;             //包含顶点和索引信息

    BoundingBox boxBound;                  //cluster自身的包围盒
    BoundingSphere sphereBound;

    uint32_t mipLevel;                     //mip层级
    BoundingSphere lodBound;               //创建该cluster的group的包围盒，没有则为自身包围盒
    float lodError;                        //创建该cluster的group的误差，没有则为0

    std::vector<uint32_t> externalEdges;   //{ clusterID, halfEdge_id }
    uint32_t groupID;                      //对group数组的下标

    void FixSize() //将三角形数目补齐到CLUSTER_SIZE
    {
        int size = mesh->index.size();
        assert(size % 3 == 0 && size > 0);
        uint32_t last = mesh->index[size - 1];

        mesh->index.resize(MeshCluster::CLUSTER_SIZE * 3);
        for (int i = size; i < mesh->index.size(); i++) mesh->index[i] = last;
    }

private:
    BeginSerailize()
    SerailizeEntry(mesh)
    SerailizeEntry(boxBound)
    SerailizeEntry(sphereBound)
    SerailizeEntry(mipLevel)
    SerailizeEntry(lodBound)
    SerailizeEntry(lodError)
    SerailizeEntry(externalEdges)
    SerailizeEntry(groupID)
    EndSerailize
};
typedef std::shared_ptr<MeshCluster> MeshClusterRef;

class MeshClusterGroup 
{
public:
    static const uint32_t GROUP_SIZE = CLUSTER_GROUP_SIZE;      //ClusterGroup内最大Cluster数目

    uint32_t mipLevel;                         //mip层级
    BoundingSphere lodBound;                   //group的包围盒，取直接构成它的全部cluster包围盒的包围盒
    float parentLodError;                     //group的误差，取直接构成它的全部cluster误差和它简化误差的最大值;
                                                //也就是说，所有用到该group生成的新cluster的cluster/group的误差都不比这个值小

    std::vector<std::pair<uint32_t, uint32_t>> externalEdges;  //{ clusterID, halfEdge_id }
    std::vector<uint32_t> clusters;                             //对cluster数组的下标

private:
    BeginSerailize()
    SerailizeEntry(mipLevel)
    SerailizeEntry(lodBound)
    SerailizeEntry(parentLodError)
    SerailizeEntry(externalEdges)
    SerailizeEntry(clusters)
    EndSerailize
};
typedef std::shared_ptr<MeshClusterGroup> MeshClusterGroupRef;

void ClusterTriangles(
    const std::shared_ptr<Mesh>& mesh,
    std::vector<MeshClusterRef>& clusters);

void GroupClusters(
    std::vector<MeshClusterRef>& clusters,
    uint32_t offset,
    uint32_t numCluster,
    std::vector<MeshClusterGroupRef>& clusterGroups,
    uint32_t mipLevel);

void BuildParentClusters(
    MeshClusterGroupRef& clusterGroup,
    std::vector<MeshClusterRef>& clusters);