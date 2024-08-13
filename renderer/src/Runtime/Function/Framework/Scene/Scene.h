#pragma once

#include "Core/Serialize/Serializable.h"
#include "Core/Util/IndexAlloctor.h"
#include "Function/Framework/Component/CameraComponent.h"
#include "Function/Framework/Entity/Entity.h"
#include "Resource/Asset/Asset.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Scene : public Asset
{
public:
    Scene() = default;
    Scene(std::string name) : name(name) { }
    ~Scene() {};

    void Tick(float deltaTime);

    std::vector<std::shared_ptr<Entity>> GetEntities() { return entities; }

    std::shared_ptr<Entity> GetEntity(std::string name);
    std::shared_ptr<Entity> GetEntity(uint32_t id);

    std::shared_ptr<Entity> CreateEntity(std::string name);
    bool AddEntity(std::shared_ptr<Entity> entity);
  
    std::shared_ptr<Entity> RemoveEntity(std::string name);
    std::shared_ptr<Entity> RemoveEntity(uint32_t id);

    std::shared_ptr<CameraComponent> GetActiveCamera();

    std::string GetName() { return name; }
    void SetName(std::string name) { this->name = name; }

protected:
    std::string name;
    std::vector<std::shared_ptr<Entity>> entities;

    IndexAlloctor idAlloctor = IndexAlloctor(UINT32_MAX);

    virtual void OnLoadAsset() override;
    virtual void OnSaveAsset() override;

private:
    BeginSerailize()
    SerailizeBaseClass(Asset)
    SerailizeEntry(name)
    SerailizeEntry(entities)
    SerailizeEntry(idAlloctor)
    EndSerailize
};