#pragma once

#include "Core/Serialize/Serializable.h"
#include "Core/Util/IndexAlloctor.h"
#include "Function/Framework/Component/CameraComponent.h"
#include "Function/Framework/Component/DirectionalLightComponent.h"
#include "Function/Framework/Component/PointLightComponent.h"
#include "Function/Framework/Component/SkyboxComponent.h"
#include "Function/Framework/Entity/Entity.h"
#include "Resource/Asset/Asset.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Scene : public Asset, public std::enable_shared_from_this<Scene>
{
public:
    Scene() = default;
    Scene(std::string name) : name(name) { }
    ~Scene() {};

    virtual std::string GetAssetTypeName() override 		{ return "Scene Asset"; }
    virtual AssetType GetAssetType() override               { return ASSET_TYPE_SCENE; }

    virtual void OnLoadAsset() override;
    virtual void OnSaveAsset() override;

    void Tick(float deltaTime);

    std::vector<std::shared_ptr<Entity>> GetEntities() { return entities; }

    std::shared_ptr<Entity> GetEntity(std::string name);
    std::shared_ptr<Entity> GetEntity(uint32_t id);

    std::shared_ptr<Entity> CreateEntity(std::string name);
    bool AddEntity(std::shared_ptr<Entity> entity);
  
    std::shared_ptr<Entity> RemoveEntity(std::string name);
    std::shared_ptr<Entity> RemoveEntity(uint32_t id);

    std::string GetName() { return name; }
    void SetName(std::string name) { this->name = name; }

    template<typename TComponent>
    std::vector<std::shared_ptr<TComponent>> GetComponents()
    {
        std::vector<std::shared_ptr<TComponent>> components;
        for(auto& entity : entities)
        {
            std::shared_ptr<TComponent> component = entity->TryGetComponent<TComponent>();
            if(component) components.push_back(component);
        }
        return components;
    }

    // 获取场景内的组件
    std::shared_ptr<CameraComponent> GetActiveCamera();
    std::shared_ptr<SkyboxComponent> GetSkyBox();
    std::shared_ptr<DirectionalLightComponent> GetDirectionalLight();
    std::vector<std::shared_ptr<PointLightComponent>> GetPointLights()  { return GetComponents<PointLightComponent>(); };

protected:
    std::string name;
    std::vector<std::shared_ptr<Entity>> entities;

    IndexAlloctor idAlloctor = IndexAlloctor(UINT32_MAX);

private:
    BeginSerailize()
    SerailizeBaseClass(Asset)
    SerailizeEntry(name)
    SerailizeEntry(entities)
    for(auto& entity : entities) entity->scene = weak_from_this();
    SerailizeEntry(idAlloctor)
    EndSerailize
};