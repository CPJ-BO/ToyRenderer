#pragma once

#include "Core/Serialize/Serializable.h"
#include "MeshCluster.h"

#include <assert.h>

//一个综合了LOD和cluster的非离散LOD(?)算法，传统的多为逐物体的离散LOD+对应的cluster
//一些相关的工作：merge-instance & cluster-culling，visibility buffer，mesh shader
//另外，虽然处理了网格的连通性问题，但是如果事先未作顶点去重（同一位置多个不同法线的顶点等），
//仍会导致即使构建group时连通了各离散三角形分量作为整体，也无法继续简化模型（简化的时候都按离散算的）


//虚拟几何体创建的主要步骤////////////////////////////////////////////////////////////////////////////////////// 
//1. 为一组三角面生成cluster              Nanite::Build::ClusterTriangles，Nanite::Build::BuildClusters
//2. 聚类cluster为cluster group           Nanite::Build::PartitionGraph
//3. 简化cluster group为新的一组三角面    Nanite::Build::DAG.Reduce


//虚拟几何体的划分////////////////////////////////////////////////////////////////////////////////////// 
//1. 单个cluster group可以由其所有的完整cluster渲染
//2. 单个cluster group可以由其部分的完整cluster和多个完整/不完整的子cluster group渲染
//3. 在2的情况时，该cluster group邻接的其他group也为2情况，来补全不完整的子cluster group
//4. 多个3中的邻接cluster group共同锁住了这一层级的边，其内部的子cluster group锁下一级的边，最终的锁边应该是一系列相互嵌套的圆环
//5. 实际参与渲染的都是cluster
//也就是：cluster group内的单个cluster可能直接渲染，也可能组合成子级的cluster gourp再细分渲染
//各级cluster group锁边是不重叠的，这样细分不会产生明显的边界


// 虚拟几何体的选择////////////////////////////////////////////////////////////////////////////////////// 
//1. 最粗暴的方法：从顶层的group节点开始进行遍历，本层的error过大就选下层的
//2. 树遍历转为数组遍历（并行）：对于每个group，存储它生成的新cluster的error，在选择时遍历group，若选择的error小于该值，说明其上层绝对不会被选择；
//                               此时再检测该group中的每个cluster，若选择的error大于cluster的error，说明该cluster是被选中的
//3. 加速：为每一层LOD的所有group建立BVH四叉树（叶节点为group，内节点为包围盒和error），再给这些四叉树再构建一个四叉树，使用类job system的方法做树遍历   （并未实现）


//TODO 现在还存在的问题////////////////////////////////////////////////////////////////////////////////////// 
//1. 面简化的库偶尔失效，手写一遍减面？
//2. 一些边界情况的处理（1.的失效问题）？
//3. 重新创建法线和切线？


// 生成顺序///////////////////////////////////////
//                   
//			                                Cluster---ClusterGroup      LOD2    
//	                    Cluster---ClusterGroup--┘				        LOD1			  			
// Cluster---ClusterGroup--┘                                           LOD0
//
/////////////////////////////////////////////

class Mesh;

class VirtualMesh 
{
public:
    void Build(MeshRef mesh);

    std::vector<std::shared_ptr<MeshCluster>> clusters;
    std::vector<std::shared_ptr<MeshClusterGroup>> clusterGroups;
    uint32_t numMipLevel;   

private:
    BeginSerailize()
    SerailizeEntry(clusters)
    SerailizeEntry(clusterGroups)
    SerailizeEntry(numMipLevel)
    EndSerailize
};

