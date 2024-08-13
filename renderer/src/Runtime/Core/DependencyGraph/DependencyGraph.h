#pragma once

#include <cstdint>
#include <map>
#include <set>
#include <vector>

// 一个低效的DAG实现
// 可以试试boost库等
class DependencyGraph
{
public:
    typedef uint32_t NodeID;
    typedef uint32_t EdgeID;

    class Edge;

    class Node
    {
    public:
        virtual ~Node() = default;

        inline NodeID ID() { return id; }

        template<typename Type = Edge>
        std::vector<Type*> InEdges() { return graph->InEdges<Type>(ID()); }

        template<typename Type = Edge>
        std::vector<Type*> OutEdges() { return graph->OutEdges<Type>(ID()); };

    private:
        NodeID id;
        DependencyGraph* graph;

        friend class DependencyGraph;
    };
    typedef Node* NodeRef;

    class Edge
    {
    public:
        virtual ~Edge() = default;

        inline EdgeID ID() { return id; }

        template<typename Type = Node>
        inline Type* From() { return graph->GetNode<Type>(from); };

        template<typename Type = Node>
        inline Type* To()   { return graph->GetNode<Type>(to); };

    private:
        EdgeID id;
        NodeID from;
        NodeID to;
        DependencyGraph* graph;

        friend class DependencyGraph;
    };
    typedef Edge* EdgeRef;

public:
    DependencyGraph() = default;
    ~DependencyGraph();

    NodeID Insert();

    void Clear();

    void Link(NodeRef from, NodeRef to, EdgeRef edge);
    
    void Remove(NodeRef node)                       { return Remove(node->ID()); };         // 删除时会自动删除相关联的边并析构
    void Remove(NodeID id);

    template<typename Type = Node, typename... Args>
    Type* CreateNode(Args&&... args) 
    {
        Type* node = new Type(std::forward<Args>(args)...);
        node->id = nodes.size();
        node->graph = this;
        nodes.push_back(node);

        outEdges[node->id] = {};
        inEdges[node->id] = {};

        return node;
    }

    template<typename Type = Edge, typename... Args>
    Type* CreateEdge(Args&&... args) 
    {
        Type* edge = new Type(std::forward<Args>(args)...);
        edge->id = edges.size();
        edge->graph = this;
        edges.push_back(edge);

        return edge;
    }

    template<typename Type = Node>
    inline Type* GetNode(NodeID id) 
    { 
        return dynamic_cast<Type*>(nodes[id]); 
    }

    template<typename Type = Edge>
    inline Type* GetEdge(EdgeID id) 
    { 
        return dynamic_cast<Type*>(edges[id]); 
    };

    template<typename Type = Edge>
    std::vector<Type*> OutEdges(NodeID id)
    {
        std::vector<Type*> edges;
        for(auto& edgeID : outEdges[id]) 
        { 
            Type* edge = dynamic_cast<Type*>(GetEdge(edgeID));
            if(edge) edges.push_back(edge); 
        }
        return  edges;
    }

    template<typename Type = Edge>
    std::vector<Type*> InEdges(NodeID id)
    {
        std::vector<Type*> edges;
        for(auto& edgeID : inEdges[id]) 
        { 
            Type* edge = dynamic_cast<Type*>(GetEdge(edgeID));
            if(edge) edges.push_back(edge); 
        }
        return  edges;
    }

    template<typename Type = Edge>
    std::vector<Type*> OutEdges(NodeRef node)     
    { 
        return OutEdges<Type>(node->ID()); 
    }

    template<typename Type = Edge>
    std::vector<Type*> InEdges(NodeRef node)      
    { 
        return InEdges<Type>(node->ID()); 
    } 

private:
    std::vector<EdgeRef> edges; // 所有的顶点和边列表，remove后可能为空
    std::vector<NodeRef> nodes;

    std::map<NodeID, std::set<EdgeID>> outEdges;
    std::map<NodeID, std::set<EdgeID>> inEdges;
};

