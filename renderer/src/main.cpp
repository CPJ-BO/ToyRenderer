
#include "Core/Math/Math.h"
#include "Function/Framework/Component/MeshRendererComponent.h"
#include "Function/Framework/Component/PointLightComponent.h"
#include "Function/Framework/Component/TransformComponent.h"
#include "Function/Framework/Component/DirectionalLightComponent.h"
#include "Function/Framework/Component/SkyboxComponent.h"
#include "Function/Global/EngineContext.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

void LoadScene()
{
    std::shared_ptr<Scene> scene = EngineContext::World()->LoadScene("Asset/BuildIn/Scene/default.asset");
    EngineContext::World()->SetActiveScene(scene->GetName());
}

void InitScene()
{
    std::shared_ptr<Scene> scene = EngineContext::World()->CreateNewScene("defaultScene");
    
    // Directional light
    if(true)
    {
        std::shared_ptr<Entity> directionalLight                                = scene->CreateEntity("Directional Light"); 
        std::shared_ptr<TransformComponent> transformComponent                  = directionalLight->TryGetComponent<TransformComponent>();
        std::shared_ptr<DirectionalLightComponent> directionalLightComponent    = directionalLight->AddComponent<DirectionalLightComponent>();

        transformComponent->SetRotation({30.0f, -60.0f, 0.0f});
    }

    // Point light
    if(true)
    {
        std::shared_ptr<Entity> pointLight                          = scene->CreateEntity("Point Light1");
        std::shared_ptr<PointLightComponent> pointLightComponent    = pointLight->AddComponent<PointLightComponent>();
        std::shared_ptr<TransformComponent> transformComponent      = pointLight->TryGetComponent<TransformComponent>();

        transformComponent->SetPosition({-25.0f, 5.0f, 8.5f});
        pointLightComponent->SetScale(15.0f);
        pointLightComponent->SetColor(Vec3(1.0f, 0.0f, 0.0f));
        pointLightComponent->SetIntencity(100.0f);
    }
    if(true)
    {
        std::shared_ptr<Entity> pointLight                          = scene->CreateEntity("Point Light2");
        std::shared_ptr<PointLightComponent> pointLightComponent    = pointLight->AddComponent<PointLightComponent>();
        std::shared_ptr<TransformComponent> transformComponent      = pointLight->TryGetComponent<TransformComponent>();

        transformComponent->SetPosition({-3.0f, 5.5f, 10.0f});
        pointLightComponent->SetScale(25.0f);
        pointLightComponent->SetColor(Vec3(0.75f, 1.0f, 0.0f));
        pointLightComponent->SetIntencity(50.0f);
    }

    // Camera
    if(true)
    {
        std::shared_ptr<Entity> camera                                  = scene->CreateEntity("Camera");
        std::shared_ptr<TransformComponent> transformComponent          = camera->TryGetComponent<TransformComponent>();
        std::shared_ptr<CameraComponent> cameraComponent                = camera->AddComponent<CameraComponent>();
        std::shared_ptr<SkyboxComponent> skyboxComponent                = camera->AddComponent<SkyboxComponent>();

        transformComponent->SetPosition({-28.0f, 2.0f, 7.0f});
        transformComponent->SetRotation({-10.0f, -5.0f, 0.0f});

        std::vector<std::string> skyboxPaths;
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/px.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/nx.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/py.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/ny.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/pz.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/nz.png");
        TextureRef skyboxTexture = std::make_shared<Texture>(skyboxPaths, TEXTURE_TYPE_CUBE);
        EngineContext::Asset()->SaveAsset(skyboxTexture, "Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR.asset");
        // TextureRef skyboxTexture = EngineContext::Asset()->GetOrLoadAsset<Texture>("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR.asset");

        skyboxComponent->SetSkyboxTexture(skyboxTexture);
    }

    // Klee
    if(true)
    {
        std::shared_ptr<Entity> model1                                  = scene->CreateEntity("Klee");
        std::shared_ptr<TransformComponent> transformComponent          = model1->TryGetComponent<TransformComponent>();
        std::shared_ptr<MeshRendererComponent> meshRendererComponent1   = model1->AddComponent<MeshRendererComponent>();

        transformComponent->SetPosition({-25.0f, 0.0f, 5.0f});
        transformComponent->SetRotation({-90.0f, 0.0f, 0.0f});
        transformComponent->SetScale({2.0f, 2.0f, 2.0f});

        ModelProcessSetting processSetting =  {
            .smoothNormal = false,
            .flipUV = false,
            .loadMaterials = false,
            .tangentSpace = true,
            .generateBVH = false,
            .generateCluster = false,
            .generateVirtualMesh = false,
            .cacheCluster = false
        };
        std::shared_ptr<Model> model = std::make_shared<Model>("Asset/BuildIn/Model/Klee/klee.fbx", processSetting);
        EngineContext::Asset()->SaveAsset(model, "Asset/BuildIn/Model/Klee/klee.asset");
        //std::shared_ptr<Model> model = EngineContext::Asset()->GetOrLoadAsset<Model>("Asset/BuildIn/Model/Klee/klee.asset");

        std::vector<TextureRef> textures1;
        textures1.push_back(std::make_shared<Texture>("Asset/BuildIn/Model/Klee/Texture/face.jpg")); 
        textures1.push_back(std::make_shared<Texture>("Asset/BuildIn/Model/Klee/Texture/hair.jpg")); 
        textures1.push_back(std::make_shared<Texture>("Asset/BuildIn/Model/Klee/Texture/dressing.jpg"));  
        EngineContext::Asset()->SaveAsset(textures1[0], "Asset/BuildIn/Model/Klee/face.asset");
        EngineContext::Asset()->SaveAsset(textures1[1], "Asset/BuildIn/Model/Klee/hair.asset");
        EngineContext::Asset()->SaveAsset(textures1[2], "Asset/BuildIn/Model/Klee/dressing.asset");  
        // textures1.push_back(EngineContext::Asset()->GetOrLoadAsset<Texture>("Asset/BuildIn/Model/Klee/face.asset")); 
        // textures1.push_back(EngineContext::Asset()->GetOrLoadAsset<Texture>("Asset/BuildIn/Model/Klee/hair.asset")); 
        // textures1.push_back(EngineContext::Asset()->GetOrLoadAsset<Texture>("Asset/BuildIn/Model/Klee/dressing.asset"));  

        std::vector<MaterialRef> materials1;
        materials1.push_back(std::make_shared<Material>());
        materials1.push_back(std::make_shared<Material>());
        materials1.push_back(std::make_shared<Material>());
        materials1[0]->SetDiffuse(textures1[0]);
        materials1[1]->SetDiffuse(textures1[1]);
        materials1[2]->SetDiffuse(textures1[2]);

        meshRendererComponent1->SetModel(model);
        meshRendererComponent1->SetMaterials(materials1);
    }

    // Bunny
    if(true)
    {
        std::shared_ptr<Entity> model2                                  = scene->CreateEntity("Bunny");
        std::shared_ptr<MeshRendererComponent> meshRendererComponent2   = model2->AddComponent<MeshRendererComponent>();
        std::shared_ptr<TransformComponent> transformComponent          = model2->TryGetComponent<TransformComponent>();

        transformComponent->SetPosition({-25.0f, 0.0f, 7.0f});
        transformComponent->SetScale({10.0f, 10.0f, 10.0f});

        // ModelProcessSetting processSetting =  {
        //     .smoothNormal = true,
        //     .flipUV = false,
        //     .loadMaterials = false,
        //     .tangentSpace = true,
        //     .generateBVH = false,
        //     .generateCluster = false,
        //     .generateVirtualMesh = true,
        //     .cacheCluster = false
        // };
        // std::shared_ptr<Model> model = std::make_shared<Model>("Asset/BuildIn/Model/Basic/stanford_bunny.obj", processSetting);
        // EngineContext::Asset()->SaveAsset(model, "Asset/BuildIn/Model/Basic/stanford_bunny.asset");
        std::shared_ptr<Model> model = EngineContext::Asset()->GetOrLoadAsset<Model>("Asset/BuildIn/Model/Basic/stanford_bunny.asset");

        MaterialRef material = std::make_shared<Material>();
        // EngineContext::Asset()->SaveAsset(material, "Asset/BuildIn/Material/Default.asset");
        // MaterialRef material = EngineContext::Asset()->GetOrLoadAsset<Material>("Asset/BuildIn/Material/Default.asset");
        // material->SetRenderQueue(995);

        meshRendererComponent2->SetModel(model);
        meshRendererComponent2->SetMaterial(material); 
    }

    // Scene
    if(true)
    {
        std::shared_ptr<Entity> model3                                  = scene->CreateEntity("Scene");
        std::shared_ptr<MeshRendererComponent> meshRendererComponent3   = model3->AddComponent<MeshRendererComponent>();
        std::shared_ptr<TransformComponent> transformComponent          = model3->TryGetComponent<TransformComponent>();

        transformComponent->SetPosition({0.0f, 0.0f, 0.0f});
        transformComponent->SetScale({0.5f, 0.5f, 0.5f});

        ModelProcessSetting processSetting =  {
            .smoothNormal = false,
            .flipUV = true,
            .loadMaterials = true,
            .tangentSpace = true,
            .generateBVH = false,
            .generateCluster = true,
            .generateVirtualMesh = false,
            .cacheCluster = false
        };
        std::shared_ptr<Model> model = std::make_shared<Model>("Asset/BuildIn/Model/Scene/scene_fix_no_tree.fbx", processSetting);
        EngineContext::Asset()->SaveAsset(model, "Asset/BuildIn/Model/Scene/sceen.asset");
        // std::shared_ptr<Model> model = EngineContext::Asset()->GetOrLoadAsset<Model>("Asset/BuildIn/Model/Scene/sceen.asset");
        
        std::vector<MaterialRef> materials3;
        for(uint32_t i = 0; i < model->GetSubmeshCount(); i++)
        {
            //materials3.push_back(std::make_shared<Material>());
            materials3.push_back(model->GetMaterial(i));
        }

        meshRendererComponent3->SetModel(model);
        meshRendererComponent3->SetMaterials(materials3);
    }

    // EngineContext::Asset()->DeleteAsset("Asset/BuildIn/Scene/default.asset");  
    EngineContext::Asset()->SaveAsset(scene, "Asset/BuildIn/Scene/default.asset");
    EngineContext::World()->SetActiveScene("defaultScene");
}

void InitStressTestScene()
{
    std::shared_ptr<Scene> scene = EngineContext::World()->CreateNewScene("stressTestScene");
    EngineContext::Render()->GetGlobalSetting()->minFrameTime = 30.0f;  // 设置一个最小帧时间
    // 主要的开销全在点光源和平行光源的阴影上了

    // Directional light
    if(true)
    {
        std::shared_ptr<Entity> directionalLight                                = scene->CreateEntity("Directional Light"); 
        std::shared_ptr<TransformComponent> transformComponent                  = directionalLight->TryGetComponent<TransformComponent>();
        std::shared_ptr<DirectionalLightComponent> directionalLightComponent    = directionalLight->AddComponent<DirectionalLightComponent>();

        transformComponent->SetRotation({30.0f, -60.0f, 0.0f});
        directionalLightComponent->SetUpdateFrequency(0, 1);
        directionalLightComponent->SetUpdateFrequency(1, 2);
        directionalLightComponent->SetUpdateFrequency(2, 5);
        directionalLightComponent->SetUpdateFrequency(3, 10);
    }

    // Point light
    if(true)
    {
        std::shared_ptr<Entity> pointLight                          = scene->CreateEntity("Point Light");
        std::shared_ptr<PointLightComponent> pointLightComponent    = pointLight->AddComponent<PointLightComponent>();
        std::shared_ptr<TransformComponent> transformComponent      = pointLight->TryGetComponent<TransformComponent>();

        transformComponent->SetPosition({-2.0f, 3.0f, 10.0f});
        pointLightComponent->SetScale(25.0f);
        pointLightComponent->SetColor(Vec3(0.5f, 0.88f, 1.0f));
        pointLightComponent->SetIntencity(50.0f);
        pointLightComponent->SetCastShadow(false);
    }

    // Camera
    if(true)
    {
        std::shared_ptr<Entity> camera                                  = scene->CreateEntity("Camera");
        std::shared_ptr<TransformComponent> transformComponent          = camera->TryGetComponent<TransformComponent>();
        std::shared_ptr<CameraComponent> cameraComponent                = camera->AddComponent<CameraComponent>();
        std::shared_ptr<SkyboxComponent> skyboxComponent                = camera->AddComponent<SkyboxComponent>();

        cameraComponent->SetFov(75.0f);

        transformComponent->SetPosition({-2.0f, 3.0f, 11.0f});
        transformComponent->SetRotation({37.0f, 5.0f, 0.0f});

        std::vector<std::string> skyboxPaths;
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/px.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/nx.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/py.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/ny.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/pz.png");
        skyboxPaths.push_back("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR/nz.png");
        TextureRef skyboxTexture = std::make_shared<Texture>(skyboxPaths, TEXTURE_TYPE_CUBE);
        EngineContext::Asset()->SaveAsset(skyboxTexture, "Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR.asset");
        // TextureRef skyboxTexture = EngineContext::Asset()->GetOrLoadAsset<Texture>("Asset/BuildIn/Texture/skybox/Factory_Sunset_Sky_Dome_LDR.asset");

        skyboxComponent->SetSkyboxTexture(skyboxTexture);
    }

    // Bunny
    MaterialRef material = std::make_shared<Material>();
    for(int i = 0; i < 1000; i++)
    {
        Vec3 pos = Vec3((i / 100) % 10, (i / 10) % 10, i % 10);

        std::shared_ptr<Entity> model2                                  = scene->CreateEntity("Bunny" + std::to_string(i));
        std::shared_ptr<MeshRendererComponent> meshRendererComponent2   = model2->AddComponent<MeshRendererComponent>();
        std::shared_ptr<TransformComponent> transformComponent          = model2->TryGetComponent<TransformComponent>();

        transformComponent->SetPosition(pos);
        transformComponent->SetScale({5.0f, 5.0f, 5.0f});

        std::shared_ptr<Model> model = EngineContext::Asset()->GetOrLoadAsset<Model>("Asset/BuildIn/Model/Basic/stanford_bunny.asset");
        meshRendererComponent2->SetModel(model);
        meshRendererComponent2->SetMaterial(material); 
    }

    EngineContext::World()->SetActiveScene("stressTestScene");
}

int main() 
{
    EngineContext::Init();

    //InitScene();
    //InitStressTestScene();
    LoadScene();

    EngineContext::MainLoop();
    EngineContext::Destroy();
    return 0;
}



