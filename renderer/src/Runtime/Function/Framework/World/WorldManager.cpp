#include "WorldManager.h"
#include "Function/Framework/Scene/Scene.h"
#include "Function/Global/EngineContext.h"
#include <memory>

void WorldManager::Init(std::string defaultScenePath)
{
    std::shared_ptr<Scene> scene = LoadScene(defaultScenePath);
    activeScene = scene;
}

void WorldManager::Tick(float deltaTime)
{
    if (activeScene) 
    {
        activeScene->Tick(deltaTime);
    }
}

std::shared_ptr<Scene> WorldManager::CreateNewScene(std::string name)
{
    std::shared_ptr<Scene> scene = std::make_shared<Scene>(name);
    scenes.push_back(scene);

    return scene;
}

std::shared_ptr<Scene> WorldManager::GetScene(std::string name)
{
    for(auto& scene : scenes)
    {
        if(scene->GetName().compare(name) == 0) return scene;
    }
    ENGINE_LOG_WARN("Can't find scene of name [%s]", name.c_str());
    return nullptr;
}

std::shared_ptr<Scene> WorldManager::SetActiveScene(std::string name)
{
    std::shared_ptr<Scene> scene = GetScene(name);
    if(scene) activeScene = scene;

    return scene;
}

std::shared_ptr<Scene> WorldManager::LoadScene(std::string path)
{
    std::shared_ptr<Scene> scene = EngineContext::Asset()->GetOrLoadAsset<Scene>(path);
    if(scene) scenes.push_back(scene);

    return scene;
}

void WorldManager::SaveScene(std::string name, const std::string filePath)
{
    std::shared_ptr<Scene> scene = GetScene(name);
    if(scene) EngineContext::Asset()->SaveAsset(scene, filePath);
}

void WorldManager::SaveScene(std::shared_ptr<Scene> scene, const std::string filePath)
{
    if(scene) EngineContext::Asset()->SaveAsset(scene, filePath);
}