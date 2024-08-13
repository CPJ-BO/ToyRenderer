
#include "Entity.h"
#include "Core/Log/log.h"

#include <memory>

void Entity::Init()
{
    for (auto& component : components)
    {
        component->Init();
    }
}

void Entity::Tick(float deltaTime)
{
    for (auto& component : components)
    {
        component->Tick(deltaTime);
    }
}

void Entity::Save()
{
    for (auto& component : components)
    {
        component->Save();
    }
}

void Entity::AddComponent(std::shared_ptr<Component> component)
{
    if(std::shared_ptr<Entity> owner = component->entity.lock())
    {
        LOG_FATAL("Component can not be bound multiple times!");
        return;
    }
    components.push_back(component);
    component->entity = weak_from_this();
}

void Entity::SetFather(std::weak_ptr<Entity> father)
{
    if(std::shared_ptr<Entity> oldFather = this->father.lock())
    {
        oldFather->RemoveChild(shared_from_this());
    }
    this->father = std::weak_ptr<Entity>(father);
    
    if(std::shared_ptr<Entity> newFather = father.lock())
    {
        newFather->children.push_back(shared_from_this());
    }
}

void Entity::AddChild(std::shared_ptr<Entity> child)
{
    children.push_back(child);
    child->SetFather(weak_from_this());
}

bool Entity::RemoveChild(std::shared_ptr<Entity> child)
{
    for (int i = 0; i < children.size(); i++)
    {
        auto& myChild = children.at(i);
        if (myChild.get() == child.get()) 
        {
            myChild->father = std::weak_ptr<Entity>();
            children.erase(children.begin() + i);
            return true;
        }
    }
    return false;
}
