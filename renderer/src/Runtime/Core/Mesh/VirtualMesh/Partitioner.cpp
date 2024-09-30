#include "partitioner.h"

#include "metis.h"

#include <assert.h>
#include <algorithm>

struct MetisGraph 
{
    idx_t nvtxs;                //顶点数目

    std::vector<idx_t> xadj;    //每个顶点的邻接边信息的偏移值
    std::vector<idx_t> adjncy;  //邻接顶点
    std::vector<idx_t> adjwgt;  //边权重
};

uint32_t PartitionGraph::Find(uint32_t x)
{
    // Find root
    uint32_t start = x;
    uint32_t root = parents[x];
    while (root != x)
    {
        x = root;
        root = parents[x];
    }

    // Point all nodes on path to root
    x = start;
    uint32_t parent = parents[x];
    while (parent != root)
    {
        parents[x] = root;
        x = parent;
        parent = parents[x];
    }

    return root;
}

uint32_t PartitionGraph::Union(uint32_t x, uint32_t y)
{
    uint32_t px = parents[x];
    uint32_t py = parents[y];

    while (px != py)
    {
        // Pick larger
        if (px < py)
        {
            parents[x] = py;
            if (x == px)
            {
                return px;
            }
            x = px;
            px = parents[x];
        }
        else
        {
            parents[y] = px;
            if (y == py)
            {
                return py;
            }
            y = py;
            py = parents[y];
        }
    }

    return px;

    
    /*
    uint32_t rootX = Find(x);
    uint32_t rootY = Find(y);
    if (rootX != rootY)
    {
        uint32_t rootMax = std::max(rootX, rootY);
        uint32_t rootMin = std::min(rootX, rootY);
        parents[rootMin] = rootMax;

        rootX = rootMax;
    }

    return rootX;
    */
}

// Optimized version of Union when iterating for( x : 0 to N ) unioning x with lower indexes.
// Neither x nor y can have already been unioned with an index > x.
uint32_t PartitionGraph::UnionSequential(uint32_t x, uint32_t y)
{
    //checkSlow(x >= y);
    //checkSlow(x == parents[x]);

    uint32_t px = x;
    uint32_t py = parents[y];
    while (px != py)
    {
        parents[y] = px;
        if (y == py)
        {
            return py;
        }
        y = py;
        py = parents[y];
    }

    return px;
}


std::shared_ptr<MetisGraph> ConvertMetisGraph(const PartitionGraph& graph)
{
    std::shared_ptr<MetisGraph> metisGraph = std::make_shared<MetisGraph>();
    metisGraph->nvtxs = graph.graph.size();

    for (auto& edge : graph.graph)
    {
        metisGraph->xadj.push_back(metisGraph->adjncy.size());
        for (auto& pair : edge)
        {
            metisGraph->adjncy.push_back(pair.first);
            metisGraph->adjwgt.push_back(pair.second);
        }
    }

    metisGraph->xadj.push_back(metisGraph->adjncy.size());

    return metisGraph;
}

void Partitioner::Init(uint32_t num_node) 
{
    nodeID.resize(num_node);
    sortTo.resize(num_node);
    uint32_t i = 0;

    for (uint32_t i = 0; i < nodeID.size(); i++) nodeID[i] = i;
    for (uint32_t i = 0; i < sortTo.size(); i++) sortTo[i] = i;
}

void Partitioner::Partition(const PartitionGraph& graph, uint32_t minPartSize, uint32_t maxPartSize)
{
    Init(graph.graph.size());
    this->minPartSize = minPartSize;
    this->maxPartSize = maxPartSize;

    std::shared_ptr<MetisGraph> graphData = ConvertMetisGraph(graph);

    RecursiveBisect(graphData, 0, graphData->nvtxs);

    sort(this->ranges.begin(), this->ranges.end());
    for (uint32_t i = 0; i < nodeID.size(); i++) 
    {
        sortTo[nodeID[i]] = i;
    }
}

void Partitioner::RecursiveBisect(std::shared_ptr<MetisGraph> graphData, uint32_t start, uint32_t end) 
{
    std::shared_ptr<MetisGraph> childGraphs[2] = { nullptr, nullptr };

    uint32_t split = Bisect(graphData, childGraphs, start, end);

    if (childGraphs[0] && childGraphs[1])             //能够划分为两个，就继续递归
    {
        RecursiveBisect(childGraphs[0], start, split);
        RecursiveBisect(childGraphs[1], split, end);
    }
    else {
        assert(
            childGraphs[0] == nullptr && 
            childGraphs[1] == nullptr);                //无法继续划分，在当前位置结束
    }
}

uint32_t Partitioner::Bisect(std::shared_ptr<MetisGraph> graphData, std::shared_ptr<MetisGraph> childGraphs[2], uint32_t start, uint32_t end)
{
    assert(end - start == graphData->nvtxs);

    if (graphData->nvtxs <= maxPartSize) //剩余待划分顶点数已不足最大单元数目，无需划分
    {
        ranges.push_back({ start,end });
        return end;
    }

    std::vector<idx_t> part(graphData->nvtxs);
    std::vector<idx_t> swapTo(graphData->nvtxs);
    {
        const uint32_t expPartSize = (minPartSize + maxPartSize) / 2;
        const uint32_t expNumParts = std::max(2u, (graphData->nvtxs + expPartSize - 1) / expPartSize);
        idx_t nw = 1;
        idx_t npart = 2;
        idx_t ncut = 0;
        real_t part_weight[] = {
            float(expNumParts >> 1) / expNumParts,
            1.0f - float(expNumParts >> 1) / expNumParts
        };

        int res = METIS_PartGraphRecursive(
            &graphData->nvtxs,                 //待划分目标（顶点）数
            &nw,                                //
            graphData->xadj.data(),            //邻接边的偏移信息
            graphData->adjncy.data(),          //邻接边数据
            nullptr,                            //顶点权重
            nullptr,                            //顶点数目
            graphData->adjwgt.data(),          //邻接边权重
            &npart,                             //划分数目
            part_weight,                        //划分权重
            nullptr,
            nullptr,                            //METIS选项
            &ncut,
            part.data()                         //划分结果，每个顶点的类别
        );
        assert(res == METIS_OK);
    }

    int left = 0, right = graphData->nvtxs - 1;
    while (left <= right)                                               //快排，存映射
    {
        while (left <= right && part[left] == 0)    swapTo[left] = left, left++;
        while (left <= right && part[right] == 1)   swapTo[right] = right, right--;
        if (left < right) 
        {
            std::swap(nodeID[start + left], nodeID[start + right]);   //
            swapTo[left] = right, swapTo[right] = left;
            left++, right--;
        }
    }
    int split = left;

    int size[2] = { split, graphData->nvtxs - split };
    assert(size[0] >= 1 && size[1] >= 1);

    if (size[0] <= maxPartSize && size[1] <= maxPartSize)   //两边都满足大小要求了，也无需继续划分
    {
        ranges.push_back({ start, start + split });
        ranges.push_back({ start + split, end });
    }
    else 
    {
        for (uint32_t i = 0; i < 2; i++)                        //还可以划分，构建子图
        {
            childGraphs[i] = std::make_shared<MetisGraph>();
            childGraphs[i]->nvtxs = size[i];
            childGraphs[i]->adjncy.reserve(graphData->adjncy.size() >> 1);
            childGraphs[i]->adjwgt.reserve(graphData->adjwgt.size() >> 1);
            childGraphs[i]->xadj.reserve(size[i] + 1);
        }
        for (uint32_t i = 0; i < graphData->nvtxs; i++) 
        {
            uint32_t index = i >= childGraphs[0]->nvtxs ? 1 : 0;
            uint32_t u = swapTo[i];

            std::shared_ptr<MetisGraph> ch = childGraphs[index];

            ch->xadj.push_back(ch->adjncy.size());
            for (uint32_t j = graphData->xadj[u]; j < graphData->xadj[u + 1]; j++) 
            {
                idx_t v = graphData->adjncy[j];
                idx_t w = graphData->adjwgt[j];
                v = swapTo[v] - (index ? size[0] : 0); //邻接顶点要重新映射，如果是右侧还得减个左侧顶点数的偏移
                if (0 <= v && v < size[index]) 
                {
                    ch->adjncy.push_back(v);
                    ch->adjwgt.push_back(w);
                }
            }
        }
        childGraphs[0]->xadj.push_back(childGraphs[0]->adjncy.size());
        childGraphs[1]->xadj.push_back(childGraphs[1]->adjncy.size());
    }
    return start + split;
}



