#pragma once

#include "Function/Render/RDG/RDGEdge.h"
#include "Function/Render/RHI/RHICommandList.h"
#include "RDGHandle.h"
#include "Core/DependencyGraph/DependencyGraph.h"
#include "Function/Render/RHI/RHIStructs.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

class RDGBuilder;

enum RDGPassNodeType
{
    RDG_PASS_NODE_TYPE_RENDER = 0,
    RDG_PASS_NODE_TYPE_COMPUTE,
    RDG_PASS_NODE_TYPE_PRESENT,

    RDG_PASS_NODE_TYPE_MAX_ENUM,    //
};

typedef struct RDGPassContext
{
    RHICommandListRef command;
    RDGBuilder* builder;
    std::array<RHIDescriptorSetRef, MAX_DESCRIPTOR_SETS> descriptors;

} RDGPassContext;

typedef std::function<void(RDGPassContext)> RDGPassExecuteFunc;

class RDGNode : public DependencyGraph::Node
{
public:
    RDGNode(std::string name) 
    : name(name)
    {}

    const std::string& Name() { return name; }

private:
    std::string name;
};
typedef RDGNode* RDGNodeRef;

// 资源节点/////////////////////////////////////////////////////////////////////////////////////

class RDGResourceNode : public RDGNode
{
public:
    RDGResourceNode(std::string name) 
    : RDGNode(name) 
    {}

    inline bool IsImported() { return isImported; }

protected:
    bool isImported = false;
};
typedef RDGResourceNode* RDGResourceNodeRef;

class RDGTextureNode : public RDGResourceNode
{
public:
    RDGTextureNode(std::string name) 
    : RDGResourceNode(name) 
    {}

    void ForEachPass(const std::function<void(RDGTextureEdgeRef, class RDGPassNode*)>& func);

    RDGTextureHandle GetHandle() { return RDGTextureHandle(ID()); } 

private:  
    RHITextureInfo info;
    RHIResourceState initState; // 从池中/外部引用时的最初状态

    RHITextureRef texture;      // 执行时分配和绑定，会动态更新，在最后一个依赖pass完成后返回资源池

    friend class RDGTextureBuilder;
    friend class RDGBuilder;
};
typedef RDGTextureNode* RDGTextureNodeRef;

class RDGBufferNode : public RDGResourceNode
{
public:
    RDGBufferNode(std::string name) 
    : RDGResourceNode(name) 
    {}

    void ForEachPass(const std::function<void(RDGBufferEdgeRef, class RDGPassNode*)>& func);

    RDGBufferHandle GetHandle() { return RDGBufferHandle(ID()); }

private:  
    RHIBufferInfo info;
    RHIResourceState initState; // 从池中/外部引用时的最初状态

    RHIBufferRef buffer;        // 执行时分配和绑定，会动态更新，在最后一个依赖pass完成后返回资源池

    friend class RDGBufferBuilder;
    friend class RDGBuilder;
};
typedef RDGBufferNode* RDGBufferNodeRef;

// pass节点/////////////////////////////////////////////////////////////////////////////////////

class RDGPassNode : public RDGNode
{
public:
    RDGPassNode(std::string name, RDGPassNodeType nodeType) 
    : RDGNode(name) 
    , nodeType(nodeType)
    {}

    inline bool Before(RDGPassNode* other)  { return ID() < other->ID(); }    // 假定所有pass的添加顺序就是执行顺序
    inline bool After(RDGPassNode* other)   { return ID() > other->ID(); }

    void ForEachTexture(const std::function<void(RDGTextureEdgeRef, RDGTextureNodeRef)>& func);
    void ForEachBuffer(const std::function<void(RDGBufferEdgeRef, RDGBufferNodeRef)>& func);

    RDGPassNodeType NodeType() { return nodeType; }

protected:
    RDGPassNodeType nodeType;
    bool isCulled = false;

    RHIRootSignatureRef rootSignature;
    std::array<RHIDescriptorSetRef, MAX_DESCRIPTOR_SETS> descriptorSets;

    std::vector<RHITextureViewRef> pooledViews;             // 动态分配的池化资源，执行完毕后返回资源池
    std::vector<std::pair<RHIDescriptorSetRef, uint32_t>> pooledDescriptorSets;
    
    friend class RDGBuilder;
};
typedef RDGPassNode* RDGPassNodeRef;


class RDGRenderPassNode : public RDGPassNode
{
public:
    RDGRenderPassNode(std::string name) 
    : RDGPassNode(name, RDG_PASS_NODE_TYPE_RENDER) 
    {}

    RDGRenderPassHandle GetHandle() { return RDGRenderPassHandle(ID()); } 
private:
    RDGPassExecuteFunc execute;

    friend class RDGRenderPassBuilder;
    friend class RDGBuilder;
};
typedef RDGRenderPassNode* RDGRenderPassNodeRef;

class RDGComputePassNode : public RDGPassNode
{
public:
    RDGComputePassNode(std::string name) 
    : RDGPassNode(name, RDG_PASS_NODE_TYPE_COMPUTE) 
    {}

    RDGComputePassHandle GetHandle() { return RDGComputePassHandle(ID()); } 
private:
    RDGPassExecuteFunc execute;

    friend class RDGComputePassBuilder;
    friend class RDGBuilder;
};
typedef RDGComputePassNode* RDGComputePassNodeRef;

class RDGPresentPassNode : public RDGPassNode
{
public:
    RDGPresentPassNode(std::string name) 
    : RDGPassNode(name, RDG_PASS_NODE_TYPE_PRESENT) 
    {}

    RDGPresentPassHandle GetHandle() { return RDGPresentPassHandle(ID()); } 
private:
    friend class RDGPresentPassBuilder;
    friend class RDGBuilder;
};
typedef RDGPresentPassNode* RDGPresentPassNodeRef;

