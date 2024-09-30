#pragma once

#include "Function/Framework/Component/CameraComponent.h"
#include "Function/Framework/Component/Component.h"
#include "Function/Framework/Component/DirectionalLightComponent.h"
#include "Function/Framework/Component/MeshRendererComponent.h"
#include "Function/Framework/Component/PointLightComponent.h"
#include "Function/Framework/Component/SkyboxComponent.h"
#include "Function/Framework/Component/TransformComponent.h"

#include <memory>

class ComponentWidget
{
public:
    static void UI(std::shared_ptr<Component> component);

private:
    static void CameraComponentUI(std::shared_ptr<CameraComponent> component);
    static void TransformComponentUI(std::shared_ptr<TransformComponent> component);
    static void DirectionalLightComponentUI(std::shared_ptr<DirectionalLightComponent> component);
    static void PointLightComponentUI(std::shared_ptr<PointLightComponent> component);
    static void MeshRendererComponentUI(std::shared_ptr<MeshRendererComponent> component);
    static void SkyboxComponentUI(std::shared_ptr<SkyboxComponent> component);
};