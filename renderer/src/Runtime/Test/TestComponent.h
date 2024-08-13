#pragma once

#include "Function/Framework/Component/MeshRendererComponent.h"
#include "Function/Framework/Component/StaticMeshComponent.h"
#include "Function/Framework/Component/TransformComponent.h"
#include "Function/Framework/Scene/Scene.h"
#include "Function/Global/EngineContext.h"

static void TestComponent()
{
    std::shared_ptr<Scene> scene = EngineContext::World()->CreateNewScene("defaultScene");
    
    scene->CreateEntity("Entity1");
    scene->CreateEntity("Entity2");

    std::shared_ptr<Entity> e1 = scene->GetEntity("Entity1");
    std::shared_ptr<Entity> e2 = scene->GetEntity("Entity2");

    std::shared_ptr<TransformComponent> transformComponent = e1->TryGetComponent<TransformComponent>();
    std::shared_ptr<CameraComponent> camera = std::make_shared<CameraComponent>();

    // 暂未完成
    // std::shared_ptr<MeshRendererComponent> meshRenderer = std::make_shared<MeshRendererComponent>();    
    // std::shared_ptr<StaticMeshComponent> staticMesh = std::make_shared<StaticMeshComponent>();

    // MaterialRef material = std::make_shared<Material>();
    // staticMesh->materials.push_back(material);

    // e2->AddComponent(meshRenderer);
    // e2->AddComponent(staticMesh);

    e1->AddComponent(camera);
    e1->TryGetComponent<CameraComponent>();
    e1->SetFather(e2);

    EngineContext::Asset()->SaveAsset(scene, "resource/build_in/config/scene/default.scene");
}