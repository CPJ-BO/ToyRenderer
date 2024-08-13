#pragma once

#include "Function/Framework/Scene/Scene.h"

#include <memory>
#include <string>
#include <vector>

class WorldManager
{
public:
    void Init(std::string defaultScenePath);

    void Tick(float deltaTime);

    std::shared_ptr<Scene> CreateNewScene(std::string name);

    const std::vector<std::shared_ptr<Scene>>& GetScenes()  { return scenes; }
    std::shared_ptr<Scene> GetScene(std::string name);
    std::shared_ptr<Scene> GetActiveScene()                 { return activeScene; }

    std::shared_ptr<Scene> SetActiveScene(std::string name);
    std::shared_ptr<Scene> LoadScene(std::string path);
    void SaveScene(std::string name, const std::string filePath = "");
    void SaveScene(std::shared_ptr<Scene> scene, const std::string filePath = "");

private:
    std::vector<std::shared_ptr<Scene>> scenes;

    std::shared_ptr<Scene> activeScene;
};

