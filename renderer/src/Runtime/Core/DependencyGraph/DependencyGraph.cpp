
#include "DependencyGraph.h"

DependencyGraph::~DependencyGraph()
{
    Clear();
}

void DependencyGraph::Clear()
{
    for(auto& node : nodes) 
    {
        if(node) delete node;
    }
    for(auto& edge : edges)
    {
        if (edge) delete edge;
    }

    nodes.clear();
    edges.clear();
}

void DependencyGraph::Link(NodeRef from, NodeRef to, EdgeRef edge)
{
    edge->from = from->ID();
    edge->to = to->ID();

    outEdges[from->ID()].insert(edge->ID());
    inEdges[to->ID()].insert(edge->ID());
}

void DependencyGraph::Remove(NodeID id)
{
    for(auto& edgeID : outEdges[id]) 
    {
        inEdges[GetEdge(edgeID)->to].erase(edgeID); // 删除出边
        delete edges[edgeID];
        edges[edgeID] = nullptr;
    }

    for(auto& edgeID : inEdges[id]) 
    {
        outEdges[GetEdge(edgeID)->to].erase(edgeID); // 删除入边
        delete edges[edgeID];
        edges[edgeID] = nullptr;
    }

    delete nodes[id];
    nodes[id] = nullptr;

    inEdges[id].clear();
    outEdges[id].clear();
}

