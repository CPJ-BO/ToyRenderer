#include "Scene.h"
#include "Function/Framework/Component/CameraComponent.h"
#include "Function/Framework/Component/TransformComponent.h"
#include "Function/Framework/Entity/Entity.h"
#include "Function/Global/EngineContext.h"
#include <memory>

CEREAL_REGISTER_TYPE(Scene)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Asset, Scene)

void Scene::OnLoadAsset()
{
    for(auto& entity : entities) 
    {
        entity->Init();
    }
}

void Scene::OnSaveAsset() 
{
    for(auto& entity : entities) 
    {
        entity->Save();
    }
}

void Scene::Tick(float deltaTime)
{
    for(auto& entity : entities) 
    {
        entity->Tick(deltaTime);
    }

    std::shared_ptr<CameraComponent> camera = GetActiveCamera();    // TODO 
    if(camera) camera->UpdateCameraInfo();
}

std::shared_ptr<Entity> Scene::GetEntity(std::string name)
{
    for(auto& entity : entities) if(entity->name.compare(name) == 0) return entity;
    return nullptr;
}

std::shared_ptr<Entity> Scene::GetEntity(uint32_t id)
{
    for(auto& entity : entities) if(entity->id == id) return entity;
    return nullptr;
}

std::shared_ptr<Entity> Scene::CreateEntity(std::string name)
{
    std::shared_ptr<Entity> entity = std::make_shared<Entity>();
    entity->name = name;
    entity->id = idAlloctor.Allocate();
    entity->AddComponent<TransformComponent>();     // 默认添加一个transform组件

    entities.push_back(entity);
    return entity;
}

bool Scene::AddEntity(std::shared_ptr<Entity> entity)
{
    if(entity->scene.lock())
    {
        ENGINE_LOG_WARN("Entity is already belonged to another scene!");
        return false;
    }

    entity->id = idAlloctor.Allocate();    // 重新分配ID
    entities.push_back(entity);
    return true;
}

std::shared_ptr<Entity> Scene::RemoveEntity(std::string name)
{
    for(int i = 0; i < entities.size(); i++)
    {
        std::shared_ptr<Entity>& entity = entities[i];
        if (entity->name.compare(name) == 0) 
        {
            entities.erase(entities.begin() + i);
            entity->scene = std::weak_ptr<Scene>();
            return entity;    // TODO 重名？
        }
    }
    return nullptr;
}

std::shared_ptr<Entity> Scene::RemoveEntity(uint32_t id)
{
    for(int i = 0; i < entities.size(); i++)
    {
        std::shared_ptr<Entity>& entity = entities[i];
        if (entity->id == id) 
        {
            entities.erase(entities.begin() + i);
            entity->scene = std::weak_ptr<Scene>();
            return entity;  
        }
    }
    return nullptr;
}

std::shared_ptr<CameraComponent> Scene::GetActiveCamera()   // TODO 暂时做成找第一个
{
    for(auto& entity : entities)
    {
        std::shared_ptr<CameraComponent> camera = entity->TryGetComponent<CameraComponent>();
        if(camera) return camera;
    }
    return nullptr;
}