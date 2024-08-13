#pragma once
#include "Component.h"
#include "Function/Framework/Entity/Entity.h"

template<typename TComponent>
std::shared_ptr<TComponent> Component::TryGetComponent()
{
    std::shared_ptr<Entity> entity;
    if((entity = this->entity.lock()))
    {
        return entity->TryGetComponent<TComponent>();
    }
}

template<typename TComponent>
std::shared_ptr<TComponent> Component::TryGetComponentInParent(bool self)
{
    std::shared_ptr<Entity> entity;
    if((entity = this->entity.lock()))
    {
        return entity->TryGetComponentInParent<TComponent>(self);
    }
}