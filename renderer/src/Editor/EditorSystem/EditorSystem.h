#pragma once

#include "Function/Framework/Entity/Entity.h"
#include <memory>
class EditorSystem 
{
public:
    EditorSystem() = default;
    ~EditorSystem() {};

    void Init();

    void UI();

    void SetSelectedEntity(std::shared_ptr<Entity> entity)  { selectedEntity = entity; }
    std::shared_ptr<Entity> GetSelectedEntity()             { return selectedEntity; }

private:
    std::shared_ptr<Entity> selectedEntity;

    bool show = true;
};