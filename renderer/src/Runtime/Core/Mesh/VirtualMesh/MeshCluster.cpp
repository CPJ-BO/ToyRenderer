
#include "MeshCluster.h"

#include "Partitioner.h"
#include "Core/Mesh/MeshOptimizor/MeshOptimizor.h"
#include "Core/Math/Hash.h"
#include "Core/Math/HashTable.h"
// #include "../bounding_box.h"
// #include "../mesh_optimizor/mesh_optimizor.h"
// #include "../../math/hash_table.h"
// #include "../../math/hash.h"

#include <assert.h>
#include <cstdint>
#include <memory>
#include <span>
#include <unordered_map>
#include <vector>

// 将原来的数位以2个0分隔：10111->1000001001001，用于生成莫顿码 
inline uint32_t ExpandBits(uint32_t v) 
{
    v = (v * 0x00010001u) & 0xFF0000FFu;
    v = (v * 0x00000101u) & 0x0F00F00Fu;
    v = (v * 0x00000011u) & 0xC30C30C3u;
    v = (v * 0x00000005u) & 0x49249249u;
    return v;
}

// 莫顿码，要求 0<=x,y,z<=1
uint32_t Morton3D(Vec3 p) 
{
    uint32_t x = p.x() * 1023;
    uint32_t y = p.y() * 1023;
    uint32_t z = p.z() * 1023;
    x = ExpandBits(x);
    y = ExpandBits(y);
    z = ExpandBits(z);
    return (x << 2) | (y << 1) | (z << 1);
}

Vec3 GetTriangleCenter(
    uint32_t triangleIndex, 
    const std::shared_ptr<Mesh>& mesh)
{
    auto& indices = mesh->index;
    auto& positions = mesh->position;

    Vec3 p0 = positions[indices[triangleIndex * 3]];
    Vec3 p1 = positions[indices[triangleIndex * 3 + 1]];
    Vec3 p2 = positions[indices[triangleIndex * 3 + 2]];

    Vec3 center = (p0 + p1 + p2) * (1.0f / 3.0f);
    return center;
}

// 莫顿码排序后在不同联通块间连边，确保网格联通
void BuildLocalityLinks(
    const std::shared_ptr<Mesh>& mesh,
    PartitionGraph& graph)
{
    auto& indices = mesh->index;
    auto& positions = mesh->position;
    uint32_t triangleSize = indices.size() / 3;

    BoundingBox bounds = { positions[indices[0]], positions[indices[0]] };
    for (auto& index : indices) bounds.Merge(positions[index]);

    Vec3 extent = bounds.maxBound - bounds.minBound;
    float maxLength = std::max(std::max(extent.x(), extent.y()), extent.z());


    std::vector<uint32_t> newTriangleIndex(triangleSize);                //莫顿码排序后的三角形的索引
    std::vector<uint32_t> sortKeys(triangleSize);
    std::vector<uint32_t> sortTo(triangleSize);                            //new_indices的反向索引
    std::vector<std::pair<uint32_t, uint32_t>> islandRuns(triangleSize);   //排序后各三角形对应的连通分量的起始和终止下标

    for(uint32_t i = 0; i < triangleSize; i++)    //计算各三角面相对位置，莫顿码编码
    {
        Vec3 center = GetTriangleCenter(i, mesh);
        Vec3 centerLocal = (center - bounds.minBound) * (1 / maxLength);  

        uint32_t morton = Morton3D(centerLocal);
        sortKeys[i] = morton;
    }

    // UE: RadixSort32排序
    // 按照莫顿码重新索引三角形序列
    for (int i = 0; i < triangleSize; i++) { newTriangleIndex[i] = i; }
    std::sort(newTriangleIndex.begin(), newTriangleIndex.end(),
        [&](uint32_t i, uint32_t j) {
            return sortKeys[i] < sortKeys[j];
        });

    for (int i = 0; i < triangleSize; i++) { sortTo[newTriangleIndex[i]] = i; }

    // Run length acceleration
    // Range of identical islandID denoting that elements are connected.
    // Used for jumping past connected elements to the next nearby disjoint element.
    {
        uint32_t runIslandID = 0;
        uint32_t runFirstElement = 0;

        for (uint32_t i = 0; i < triangleSize; i++)
        {
            uint32_t islandID = graph.Find(newTriangleIndex[i]); //找到该三角形所属的连通分量

            if (runIslandID != islandID)
            {
                // We found the end so rewind to the beginning of the run and fill.
                for (uint32_t j = runFirstElement; j < i; j++)
                {
                    islandRuns[j].second = i - 1;
                }

                // Start the next run
                runIslandID = islandID;
                runFirstElement = i;
            }

            islandRuns[i].first = runFirstElement;
        }
        // Finish the last run
        for (uint32_t j = runFirstElement; j < triangleSize; j++)
        {
            islandRuns[j].second = triangleSize - 1;
        }
    }

    int count = 0;
    for (uint32_t i = 0; i < triangleSize; i++)
    {
        uint32_t index = newTriangleIndex[i];

        uint32_t runLength = islandRuns[i].second - islandRuns[i].first + 1;
        //if (runLength < 128)
        {         
            Vec3 center = GetTriangleCenter(index, mesh);
            uint32_t islandID = graph.parents[index];

            const uint32_t maxLinksPerElement = 5;

            uint32_t closestindex[maxLinksPerElement];
            float  closestDist2[maxLinksPerElement];
            for (int k = 0; k < maxLinksPerElement; k++)
            {
                closestindex[k] = ~0u;
                closestDist2[k] = 3.402823466e+38F;
            }

            for (int direction = 0; direction < 2; direction++)
            {
                uint32_t limit = direction ? triangleSize - 1 : 0;
                uint32_t Step = direction ? 1 : -1;

                uint32_t adj = i;
                for (int iterations = 0; iterations < 16; iterations++)
                {
                    if (adj == limit)
                        break;
                    adj += Step;

                    uint32_t adjIndex = newTriangleIndex[adj];
                    uint32_t adjIslandID = graph.parents[adjIndex];

                    if (islandID == adjIslandID)
                    {
                        // Skip past this run
                        if (direction)
                            adj = islandRuns[adj].second;
                        else
                            adj = islandRuns[adj].first;
                    }
                    else
                    {
                        // Add to sorted list
                        float adjDist2 = pow((center - GetTriangleCenter(adjIndex, mesh)).norm(), 2);
                        for (int k = 0; k < maxLinksPerElement; k++)
                        {
                            if (adjDist2 < closestDist2[k])
                            {
                                std::swap(adjIndex, closestindex[k]);
                                std::swap(adjDist2, closestDist2[k]);
                            }
                        }
                    }
                }
            }
     
            for (int k = 0; k < maxLinksPerElement; k++)
            {
                if (closestindex[k] != ~0u)
                {
                    // Add both directions
                    graph.IncreaseEdgeCost(index, closestindex[k], 1);
                    graph.IncreaseEdgeCost(closestindex[k], index, 1);
                    //printf("[%d, %d]\n", index, closestindex[k]);
                    count++;
                }
            }
            
        }
    }
    //if (count > 0) printf("BuildLocalityLinks create %d new links, %d triangles total\n", count, triangleSize);

    uint32_t island = graph.Find(0);
    for (int i = 0; i < triangleSize; i++) assert(graph.Find(i) == island);

}


// step 1. 为一组三角面生成cluster////////////////////////////////////////////////////////////////

// 使用半边结构来构建模型的
// 半边邻接图和三角面邻接图
inline uint32_t Cycle3(uint32_t i) {
    uint32_t imod3 = i % 3;
    return i - imod3 + ((1 << imod3) & 3);
}
inline uint32_t Cycle3(uint32_t i, uint32_t ofs) {
    return i - i % 3 + (i + ofs) % 3;
}

// 边哈希，找到共享顶点且相反的边，代表两三角形相邻
void BuildEdgeGraph(
    const std::shared_ptr<Mesh>& mesh,
    PartitionGraph& edgeGraph) 
{
    auto& indices = mesh->index;
    auto& positions = mesh->position;
    uint32_t indexCount = indices.size();

    HashTable edgeHash(indexCount);
    edgeGraph.Init(indexCount);

    for (uint32_t i = 0; i < indexCount; i++)
    {
        Vec3 p0 = positions[indices[i]];
        Vec3 p1 = positions[indices[Cycle3(i)]];    //每条边作为半边被加入两次，每个顶点索引也对应了一个半边，每三个相邻索引对应同一个三角形
        //Vertex p0 = vertices[indices[i]];
        //Vertex p1 = vertices[indices[Cycle3(i)]];
        edgeHash.Add(Hash(p0, p1), i);

        for (uint32_t j : edgeHash[Hash(p1, p0)])         //三角形的手性是一致的，则若查找到了共享顶点且相反的半边
        {                                                   //说明该边邻接着两个三角面，更新权重以将这两个边互相索引
            if (p1 == positions[indices[j]] &&
                p0 == positions[indices[Cycle3(j)]])
            {
                edgeGraph.IncreaseEdgeCost(i, j, 1);
                edgeGraph.IncreaseEdgeCost(j, i, 1);
            }
        }
    }
}

// 根据半边的邻接构建三角形的邻接图，边权为1，当需要加入local时需要adjacency边权足够大
void BuildTriangleGraph(
    const PartitionGraph& edgeGraph,
    PartitionGraph& triangleGraph) 
{
    triangleGraph.Init(edgeGraph.graph.size() / 3);
    uint32_t index = 0;
    for (const auto& mp : edgeGraph.graph)
    {
        for (auto& pair : mp) 
        {
            triangleGraph.IncreaseEdgeCost(index / 3, pair.first / 3, 1);  //对于关联的负半边，根据索引找到对应的三角形，建立邻接
        }
        index++;
    }
}

void ClusterTriangles(
    const std::shared_ptr<Mesh>& mesh,
    std::vector<MeshClusterRef>& clusters)
{
    auto& indices = mesh->index;
    auto& positions = mesh->position;

    PartitionGraph edgeGraph, triangleGraph;
    BuildEdgeGraph(mesh, edgeGraph);                       //构建边的邻接图
    BuildTriangleGraph(edgeGraph, triangleGraph);         //构建三角形的邻接图 
    //BuildLocalityLinks(mesh, indices, triangleGraph);        //保证各分量连通

    Partitioner partitioner;
    partitioner.Partition(triangleGraph, MeshCluster::CLUSTER_SIZE - 4, MeshCluster::CLUSTER_SIZE);

    // 根据划分结果构建clusters
    for (auto& range : partitioner.ranges) 
    {
        uint32_t left = range.first;
        uint32_t right = range.second;

        MeshClusterRef cluster = std::make_shared<MeshCluster>();
        clusters.push_back(cluster);

        std::vector<uint32_t> subMeshIndex;
        for (uint32_t i = left; i < right; i++)
        {
            uint32_t triangleIndex = partitioner.nodeID[i];
            for (uint32_t k = 0; k < 3; k++) 
            {
                uint32_t halfEdgeIndex = triangleIndex * 3 + k;
                uint32_t vertexIndex = indices[halfEdgeIndex];

                bool isExternal = false;
                for (auto& pair : edgeGraph.graph[halfEdgeIndex])
                {
                    uint32_t adjEdgeIndex = pair.first;

                    uint32_t adjTriangle = partitioner.sortTo[adjEdgeIndex / 3];       // sortTo[nodeID[i]] = i，反向映射，方便从变换后的三角形图查找未变换的半边图
                    if (adjTriangle < left || adjTriangle >= right)                         // 邻接三角形索引不在该聚类范围，说明此处为cluster边缘
                    { 
                        isExternal = true;
                        break;
                    }
                }
                if (isExternal) 
                {
                    cluster->externalEdges.push_back(subMeshIndex.size());
                }
                subMeshIndex.push_back(vertexIndex);
            }
        }
        cluster->mesh = std::make_shared<Mesh>(*mesh.get(), subMeshIndex);

        cluster->mipLevel = 0;
        cluster->lodError = 0;
        cluster->sphereBound = BoundingSphere(cluster->mesh->position);
        cluster->lodBound = cluster->sphereBound;
        cluster->boxBound = BoundingBox(cluster->mesh->aabb);
    }
}

// step 2. 聚类cluster为cluster group////////////////////////////////////////////////////////////////

void BuildClustersEdgeGraph(
    std::vector<MeshClusterRef> clusters,
    const std::vector<std::pair<uint32_t, uint32_t>>& externalEdges,
    PartitionGraph& edgeGraph) 
{
    HashTable edgeHash(externalEdges.size());
    edgeGraph.Init(externalEdges.size());

    uint32_t i = 0;
    for (auto& pair : externalEdges) 
    {
        uint32_t clusterID = pair.first;
        uint32_t originEdgeID = pair.second;

        auto& indices = clusters[clusterID]->mesh->index;
        auto& positions = clusters[clusterID]->mesh->position;

        Vec3 p0 = positions[indices[originEdgeID]];
        Vec3 p1 = positions[indices[Cycle3(originEdgeID)]];
        edgeHash.Add(Hash(p0, p1), i);

        for (uint32_t j : edgeHash[Hash(p1, p0)])     // 和BuildEdgeGraph中基本一致，只是需要额外通过cluster索引原边界半边的信息
        {
            auto& pair1 = externalEdges[j];

            uint32_t otherClusterID = pair1.first;
            uint32_t otherOriginEdgeID = pair1.second;

            auto& indices1 = clusters[otherClusterID]->mesh->index;
            auto& positions1 = clusters[otherClusterID]->mesh->position;

            if (positions1[indices1[otherOriginEdgeID]] == p1 &&
                positions1[indices1[Cycle3(otherOriginEdgeID)]] == p0)
            {
                edgeGraph.IncreaseEdgeCost(i, j, 1);
                edgeGraph.IncreaseEdgeCost(j, i, 1);
            }
        }
        i++;
    }
}

void BuildClustersGraph(
    const PartitionGraph& edgeGraph,
    const std::vector<uint32_t>& mp,
    uint32_t numCluster,
    PartitionGraph& graph) 
{
    graph.Init(numCluster);
    uint32_t index = 0;
    for (const auto& emp : edgeGraph.graph)
    {
        for (auto& pair : emp) 
        {
            graph.IncreaseEdgeCost(mp[index], mp[pair.first], 1);   // 和BuildTriangleGraph中基本一致，也是找关联半边建立邻接
        }
        index++;
    }
}

void GroupClusters(
    std::vector<MeshClusterRef>& clusters,
    uint32_t offset,
    uint32_t numCluster,
    std::vector<MeshClusterGroupRef>& clusterGroups,
    uint32_t mipLevel)
{
    std::vector<MeshClusterRef> subClusters;
    for (int i = 0; i < numCluster; i++) subClusters.push_back(clusters[offset + i]);

    // 取出每个cluster的边界，并建立边id到簇id的映射
    std::vector<uint32_t> mp;                               // 合并后各边界半边的索引到其cluster的索引
    std::vector<uint32_t> mp1;                              // cluster到其首个合并后边界半边的索引偏移
    std::vector<std::pair<uint32_t, uint32_t>> externalEdges;   // 合并后各边界半边的索引到其cluster的索引和其在cluster内的原索引
    uint32_t i = 0;
    for (auto& cluster : subClusters)
    {
        assert(cluster->mipLevel == mipLevel);
        mp1.push_back(mp.size());
        for (uint32_t halfEdge : cluster->externalEdges) 
        {
            externalEdges.push_back({ i, halfEdge });
            mp.push_back(i);
        }
        i++;
    }

    PartitionGraph edgeGraph, graph;
    BuildClustersEdgeGraph(subClusters, externalEdges, edgeGraph);
    BuildClustersGraph(edgeGraph, mp, numCluster, graph);

    Partitioner partitioner;
    partitioner.Partition(graph, MeshClusterGroup::GROUP_SIZE - 4, MeshClusterGroup::GROUP_SIZE);

    for (auto& range : partitioner.ranges)
    {
        uint32_t left = range.first;
        uint32_t right = range.second;
    
        auto group = std::make_shared<MeshClusterGroup>();
        clusterGroups.push_back(group);
        group->mipLevel = mipLevel;

        for (uint32_t i = left; i < right; i++)
        {
            uint32_t clusterID = partitioner.nodeID[i];
            clusters[clusterID + offset]->groupID = clusterGroups.size() - 1;
            group->clusters.push_back(clusterID + offset);
            for (uint32_t halfEdgeIndex = mp1[clusterID]; halfEdgeIndex < mp.size() && mp[halfEdgeIndex] == clusterID; halfEdgeIndex++) 
            {
                bool isExternal = false;
                for (auto& pair : edgeGraph.graph[halfEdgeIndex])
                {
                    uint32_t adjEdge = pair.first;
                    uint32_t adjCluster = partitioner.sortTo[mp[adjEdge]];

                    if (adjCluster < left || adjCluster >= right)
                    {
                        isExternal = true;
                        break;
                    }
                }
                if (isExternal) 
                {
                    uint32_t e = externalEdges[halfEdgeIndex].second;
                    group->externalEdges.push_back({ clusterID + offset, e });
                }
            }
        }
    }
}

// step 3. 简化cluster group为新的一组三角面，并继续创建cluster////////////////////////////////////////////////////////////////

void BuildParentClusters(
    MeshClusterGroupRef& clusterGroup,
    std::vector<MeshClusterRef>& clusters)
{
    // 收集子cluster信息
    std::vector<BoundingSphere> lodBound;
    float parentLodError = 0;
    MeshRef tempMesh = std::make_shared<Mesh>();
    std::vector<uint32_t> tempIndex;
    for(auto& clusterID : clusterGroup->clusters)
    {
        auto& cluster = clusters[clusterID];

        lodBound.push_back(cluster->lodBound);
        parentLodError = std::max(parentLodError, cluster->lodError); //强制父节点的error大于等于子节点
        tempMesh->Merge(*cluster->mesh.get());
    }
    BoundingSphere parentLodBound = BoundingSphere(lodBound);

    // 简化网格  
    MeshOptimizor::RemapMesh(tempMesh);  //子cluster各组还有重复的顶点要去除
    uint32_t prevIndexSize = tempMesh->index.size();
    float error = MeshOptimizor::SimplifyMesh(tempMesh, 0.3f, 1e50, tempIndex);        //减面
    if ((float)tempIndex.size() / prevIndexSize > 0.5)
    {
        printf("ERROR SimplifyMesh at level [%d], rate [%f], indexCnt [%d], try to do it sloppy...\n", clusterGroup->mipLevel + 1, (float)tempIndex.size() / prevIndexSize, prevIndexSize);
        
        float error = MeshOptimizor::SimplifyMeshSloppy(tempMesh, 0.3f, 1e50, tempIndex);  //TODO 减面算法还是有问题，减不下去了就用sloppy的？
        
        assert((float)tempIndex.size() / prevIndexSize < 0.5);
        if (tempIndex.size() == 0) for (int i = 0; i < 3; i++) tempIndex.push_back(tempMesh->index[0]);
    }
    //MeshOptimizor::RemapMesh(tempMesh); 
    parentLodError = std::max(parentLodError, sqrt(error));


    MeshRef parentMesh = std::make_shared<Mesh>(*tempMesh.get(), tempIndex);
   // 生成新cluster
    std::vector<MeshClusterRef> parentClusters;
    ClusterTriangles(parentMesh, parentClusters);

    //强制父节点的lod包围盒覆盖所有子节点lod包围盒
    for (auto& cluster : parentClusters)
    {
        cluster->mipLevel = clusterGroup->mipLevel + 1;
        cluster->lodBound = parentLodBound;                 
        cluster->lodError = parentLodError;      
    }

    clusterGroup->lodBound = parentLodBound;
    clusterGroup->parentLodError = parentLodError;

    for (auto& cluster : parentClusters) clusters.push_back(cluster);
}

