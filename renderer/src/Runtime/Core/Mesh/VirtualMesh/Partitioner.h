#pragma once

#include <vector>
#include <map>
#include <queue>
#include <set>
#include <memory>

struct MetisGraph;

class PartitionGraph //UE使用的FDisjointSet是并查集
{
public:
    std::vector<std::map<uint32_t, int>> graph; //顶点 -> {顶点, 权重} 的映射表
    std::vector<uint32_t> parents;              //并查集

    inline void Init(uint32_t n)                                            { graph.resize(n);  for (int i = 0; i < n; i++) parents.push_back(parents.size()); }
    inline void AddNode()                                                   { graph.push_back({}); parents.push_back(parents.size());}
    inline void AddEdge(uint32_t from, uint32_t to, int cost)               { graph[from][to] = cost;   Union(from, to); }
    inline void IncreaseEdgeCost(uint32_t from, uint32_t to, int cost)      { graph[from][to] += cost;  Union(from, to); }

    uint32_t Find(uint32_t x);
    uint32_t Union(uint32_t x, uint32_t y);
    uint32_t UnionSequential(uint32_t x, uint32_t y);
};

class Partitioner 
{ 
public:
    void Init(uint32_t num_node);
    void Partition(const PartitionGraph& graph, uint32_t minPartSize, uint32_t maxPartSize);

    std::vector<std::pair<uint32_t, uint32_t>> ranges;  //划分范围 [起始索引，结束索引]
    std::vector<uint32_t> nodeID;                      //节点映射，在划分范围内的为同一划分
    std::vector<uint32_t> sortTo;                      //反向映射，以保证可以查询映射前的对应信息
    uint32_t minPartSize;                             //最小的单聚类节点数
    uint32_t maxPartSize;                             //最大的单聚类节点数

private:
    void RecursiveBisect(std::shared_ptr<MetisGraph> graphData, uint32_t start, uint32_t end);

    uint32_t Bisect(std::shared_ptr<MetisGraph> graphData, std::shared_ptr<MetisGraph> childGraphs[2], uint32_t start, uint32_t end);

};