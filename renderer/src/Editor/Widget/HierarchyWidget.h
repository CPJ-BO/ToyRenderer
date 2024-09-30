#pragma once

#include "Function/Framework/Entity/Entity.h"

#include <memory>

class HierarchyWidget
{
public:
    static void UI();

private:
    static void EntityUI(std::shared_ptr<Entity> entity);
};