#include "RDGNode.h"

void RDGTextureNode::ForEachPass(const std::function<void(RDGTextureEdgeRef, RDGPassNodeRef)>& func)
{
    for(auto& edge : InEdges<RDGTextureEdge>())     func(edge, edge->From<RDGPassNode>());
    for(auto& edge : OutEdges<RDGTextureEdge>())    func(edge, edge->To<RDGPassNode>());
}

void RDGBufferNode::ForEachPass(const std::function<void(RDGBufferEdgeRef, class RDGPassNode*)>& func)
{
    for(auto& edge : InEdges<RDGBufferEdge>())     func(edge, edge->From<RDGPassNode>());
    for(auto& edge : OutEdges<RDGBufferEdge>())    func(edge, edge->To<RDGPassNode>());
}

void RDGPassNode::ForEachTexture(const std::function<void(RDGTextureEdgeRef, RDGTextureNodeRef)>& func)
{
    std::vector<RDGTextureEdgeRef> inTextures = InEdges<RDGTextureEdge>();
    std::vector<RDGTextureEdgeRef> outTextures = OutEdges<RDGTextureEdge>();

    for(auto& textureEdge : inTextures)     func(textureEdge, textureEdge->From<RDGTextureNode>());
    for(auto& textureEdge : outTextures)    func(textureEdge, textureEdge->To<RDGTextureNode>());
}

void RDGPassNode::ForEachBuffer(const std::function<void(RDGBufferEdgeRef, RDGBufferNodeRef)>& func)
{
    std::vector<RDGBufferEdgeRef> inBuffers = InEdges<RDGBufferEdge>();
    std::vector<RDGBufferEdgeRef> outBuffers = OutEdges<RDGBufferEdge>();

    for(auto& bufferEdge : inBuffers)   func(bufferEdge, bufferEdge->From<RDGBufferNode>());
    for(auto& bufferEdge : outBuffers)  func(bufferEdge, bufferEdge->To<RDGBufferNode>());
}