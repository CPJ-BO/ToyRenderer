#pragma once

#include "Core/Serialize/Serializable.h"
#include "Function/Framework/Component/Component.h"

#include <vector>
#include <string>
#include <memory>

class Scene;

class Entity : public std::enable_shared_from_this<Entity> 
{
public:
    void Load();
    void Save();
    void Init();
    void Tick(float deltaTime);

    template<typename TComponent>
    std::shared_ptr<TComponent> TryGetComponent()
    {
        for (auto& component : components)
        {
            std::shared_ptr<TComponent> cast = std::dynamic_pointer_cast<TComponent>(component);
            if(cast != nullptr) return cast;
        }
        return nullptr;
    }

    template<typename TComponent>
    std::shared_ptr<TComponent> TryGetComponentInParent(bool self = false)
    {   
        if(self)
        {
            std::shared_ptr<TComponent> component = TryGetComponent<TComponent>();
            if(component) return component;
        }

        std::shared_ptr<Entity> entity = father.lock();
        if(entity) return entity->TryGetComponentInParent<TComponent>(true);

        return nullptr;
    }

    template<typename TComponent>
    bool RemoveComponent()
    {
        for (int i = 0; i < components.size(); i++)
        {
            auto& component = components.at(i);

            std::shared_ptr<TComponent> cast = std::dynamic_pointer_cast<TComponent>(component);
            if(cast != nullptr) 
            {
                components.erase(components.begin() + i);
                return true;
            }
        }
        return false;
    }

    template <class TComponent, class... Args>
    std::shared_ptr<TComponent> AddComponent(Args&&... args)
    {
        std::shared_ptr<TComponent> component = std::make_shared<TComponent>(args...);
        AddComponent(component);

        return component;
    }

    void AddComponent(std::shared_ptr<Component> component);
    std::vector<std::shared_ptr<Component>>& GetComponents()        { return components; }

    inline uint32_t GetID()                                         { return id; }
    inline std::string GetName()                                    { return name; }
    inline void SetName(std::string name)                           { this->name = name; }
    inline std::weak_ptr<Entity> GetFather()                        { return father; }
    inline std::vector<std::shared_ptr<Entity>> GetChildren()       { return children; }

    void SetFather(std::weak_ptr<Entity> father);
    void AddChild(std::shared_ptr<Entity> child);
    bool RemoveChild(std::shared_ptr<Entity> child);

    inline std::shared_ptr<Scene> GetScene()                          { return scene.lock(); }
    
private:
    uint32_t id = 0;    // 运行时分配，不做序列化
    std::string name = "";
    std::vector<std::shared_ptr<Component>> components;

    std::weak_ptr<Entity> father;
    std::vector<std::shared_ptr<Entity>> children;

    std::weak_ptr<Scene> scene;

private:
    BeginSerailize()
    SerailizeEntry(name)
    SerailizeEntry(components)
    SerailizeEntry(children)
    for(auto& component : components) component->entity = weak_from_this();
    for(auto& child : children) child->father = weak_from_this();
    EndSerailize 

    friend class Scene; // 由Scene负责创建Entity 
};