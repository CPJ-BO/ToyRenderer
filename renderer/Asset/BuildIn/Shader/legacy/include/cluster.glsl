#ifndef CLUSTER_GLSL
#define CLUSTER_GLSL


// 获取实例索引
uint fetchClusterInstanceIndex(uint indirectBufferID, uint instanceIndex) 
{
    ClusterIndex clusterIndex = DRAW_CLUSTERS[indirectBufferID].data.indices[instanceIndex];

    return clusterIndex.instanceIndex;
}

// 获取cluster索引
uint fetchClusterID(uint indirectBufferID, uint instanceIndex)
{
    ClusterIndex clusterIndex = DRAW_CLUSTERS[indirectBufferID].data.indices[instanceIndex];

    return clusterIndex.clusterID;
}

// 获取一个Cluster的标识，直接用instanceIndex每帧是不同的
// uint fetchClusterUniqueID(uint indirectBufferID, uint instanceIndex)
// {
//     ClusterIndex clusterIndex = DRAW_CLUSTERS[indirectBufferID].data.indices[instanceIndex];
//     ClusterInfo clusterInfo = CLUSTERS.slot[clusterIndex.clusterID];

//     return clusterIndex.instanceIndex << 8 + clusterIndex.clusterID;
// }


// 获取物体信息
Object fetchClusterObject(uint indirectBufferID, uint instanceIndex) 
{
    Object object = OBJECTS.slot[fetchClusterInstanceIndex(indirectBufferID, instanceIndex)];

    return object;
}

// 获取顶点信息
Vertex fetchClusterVertex(uint indirectBufferID, uint vertexIndex, uint instanceIndex)
{
    ClusterIndex clusterIndex = DRAW_CLUSTERS[indirectBufferID].data.indices[instanceIndex];
    ClusterInfo clusterInfo = CLUSTERS.slot[clusterIndex.clusterID];

    uint idx = INDICES[clusterInfo.indexID].slot[vertexIndex];
    Vertex vert = VERTICES[clusterInfo.vertexID].slot[idx];

    return vert;
}

uint fetchClusterTriangleID(uint indirectBufferID, uint vertexIndex, uint instanceIndex)
{
    ClusterIndex clusterIndex = DRAW_CLUSTERS[indirectBufferID].data.indices[instanceIndex];
    ClusterInfo clusterInfo = CLUSTERS.slot[clusterIndex.clusterID];

    uint idx = INDICES[clusterInfo.indexID].slot[vertexIndex];
    return idx / 3;
}

#endif