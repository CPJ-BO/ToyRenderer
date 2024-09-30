#include "RenderLightManager.h"
#include "Core/Math/BoundingBox.h"
#include "Function/Global/Definations.h"
#include "Function/Global/EngineContext.h"
#include <array>
#include <cstdint>

void RenderLightManager::Init()
{

}

void RenderLightManager::Tick()
{
    PrepareLights();
}

void RenderLightManager::PrepareLights()
{
    ENGINE_TIME_SCOPE(RenderLightManager::PrepareLights);

    LightSetting setting = {};
    setting.directionalLightCnt = 0;
    setting.pointLightCnt = 0;
    setting.pointshadowedLightCnt = 0;
    setting.volumeLightCnt = 0;
    setting.globalIndexOffset = 0;

    pointShadowLights.clear();
    directionalLight = nullptr;

    // 收集光源信息,更新参数
    // TODO 场景的CPU端剔除

    directionalLight = EngineContext::World()->GetActiveScene()->GetDirectionalLight();
    if(directionalLight) directionalLight->UpdateLightInfo();
    setting.directionalLightCnt = directionalLight ? 1 : 0;

    auto pointLightComponents = EngineContext::World()->GetActiveScene()->GetPointLights();
    for(auto& pointLight : pointLightComponents) 
    {
        if(pointLight) 
        {
            pointLight->pointShadowID = MAX_POINT_SHADOW_COUNT; 

            if(pointLight->Enable() &&
               pointLight->CastShadow() && 
               setting.pointshadowedLightCnt < MAX_POINT_SHADOW_COUNT)                 // 暂时取前几个
            {                                                                           // TODO 可以做更新策略，阴影分帧更新等
                pointLight->pointShadowID = setting.pointshadowedLightCnt++;
                setting.pointShadowLightIDs[pointLight->pointShadowID] = pointLight->pointLightID;
                pointShadowLights.push_back(pointLight);
            }
            if(pointLight->Enable())
            {
                setting.pointLightIDs[setting.pointLightCnt] = pointLight->pointLightID;  
                setting.pointLightCnt++;        
            }
            pointLight->UpdateLightInfo();
        }
    }

    // 提交整体处理的信息
    EngineContext::RenderResource()->SetLightSetting(setting);
}