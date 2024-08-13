#pragma once

#include "Core/DependencyGraph/DependencyGraph.h"

#include <cstdint>

using NodeID = DependencyGraph::NodeID;

class RDGResoruceHandle
{
public:
    RDGResoruceHandle(NodeID id) : id(id) {}

    bool operator< (const RDGResoruceHandle& other) const noexcept {
        return id < other.id;
    }

    bool operator== (const RDGResoruceHandle& other) const noexcept {
        return (id == other.id);
    }

    bool operator!= (const RDGResoruceHandle& other) const noexcept {
        return !operator==(other);
    }

    inline NodeID ID() { return id; }

protected:
    NodeID id = UINT32_MAX;
};

class RDGPassHandle : public RDGResoruceHandle
{
public:
    RDGPassHandle(NodeID id) : RDGResoruceHandle(id) {};
};

class RDGRenderPassHandle : public RDGPassHandle
{
public:
    RDGRenderPassHandle(NodeID id) : RDGPassHandle(id) {};
};

class RDGComputePassHandle : public RDGPassHandle
{
public:
    RDGComputePassHandle(NodeID id) : RDGPassHandle(id) {};
};

class RDGCopyPassHandle : public RDGPassHandle
{
public:
    RDGCopyPassHandle(NodeID id) : RDGPassHandle(id) {};
};

class RDGPresentPassHandle : public RDGPassHandle
{
public:
    RDGPresentPassHandle(NodeID id) : RDGPassHandle(id) {};
};

class RDGTextureHandle : public RDGResoruceHandle
{
public:
    RDGTextureHandle(NodeID id) : RDGResoruceHandle(id) {};
};

class RDGBufferHandle : public RDGResoruceHandle
{
public:
    RDGBufferHandle(NodeID id) : RDGResoruceHandle(id) {};
};
