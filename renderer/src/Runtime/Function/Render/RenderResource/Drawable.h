#pragma once

#include "Function/Render/RenderPass/MeshPass.h"

class Drawable
{
public:
    virtual void CollectDrawBatch(std::vector<DrawBatch>& batches) = 0;
};