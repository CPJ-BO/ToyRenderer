#pragma once

#include "Function/Framework/Component/DirectionalLightComponent.h"
#include "Function/Framework/Component/PointLightComponent.h"
#include <memory>
#include <vector>
class RenderLightManager
{
public:
    void Init();
    void Tick();

    inline std::shared_ptr<DirectionalLightComponent> GetDirectionalLight()                 { return directionalLight; }
    inline const std::vector<std::shared_ptr<PointLightComponent>>& GetPointShadowLights()  { return pointShadowLights; }

private:
    void PrepareLights();

    std::vector<std::shared_ptr<PointLightComponent>> pointShadowLights;
    std::shared_ptr<DirectionalLightComponent> directionalLight;
};