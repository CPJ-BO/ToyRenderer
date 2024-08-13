#pragma once

#include "Core/Math/Math.h"
#include "Function/Framework/Component/TransformComponent.h"

#include "GLFW/glfw3.h"
#include <iostream>
#include <ostream>

static void KeyMove(std::shared_ptr<TransformComponent> transformComponent, float deltaTime, GLFWwindow* window)
{
    float speed = 2.5f;
    float delta = speed * deltaTime / 1000.0f;
    Vec3 deltaPosition = Vec3::Zero();

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		deltaPosition += transformComponent->GetTransform().Front() * delta;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		deltaPosition -= transformComponent->GetTransform().Front() * delta;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		deltaPosition -= transformComponent->GetTransform().Right() * delta;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		deltaPosition += transformComponent->GetTransform().Right() * delta;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		deltaPosition += transformComponent->GetTransform().Up() * delta;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		deltaPosition -= transformComponent->GetTransform().Up() * delta;

    transformComponent->Translate(deltaPosition);
}

static void MouseMove(std::shared_ptr<TransformComponent> transformComponent, float deltaTime, GLFWwindow* window)
{
    float sensitivity = 0.5f;

	//if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) return;
	//if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) return;

    Vec2 offset = Vec2::Zero();
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) offset.y() = deltaTime * sensitivity;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) offset.y() = -deltaTime * sensitivity;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) offset.x() = -deltaTime * sensitivity;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) offset.x() = deltaTime * sensitivity;

    // Vec3 eulerAngle = transformComponent->GetTransform().GetEulerAngle();    //yaw pitch roll

	// eulerAngle.x() += offset.x();
	// eulerAngle.x() = eulerAngle.x() > 180 ? eulerAngle.x() - 360 : eulerAngle.x();
	// eulerAngle.x() = eulerAngle.x() < -180 ? eulerAngle.x() + 360 : eulerAngle.x();

	// eulerAngle.y() += offset.y();
	// if (eulerAngle.y() > 89.0f)
	// 	eulerAngle.y() = 89.0f;
	// if (eulerAngle.y() < -89.0f)
	// 	eulerAngle.y() = -89.0f;

    // transformComponent->SetRotation(eulerAngle);

    transformComponent->Rotate(Vec3(offset.x(), offset.y(), 0.0f));

}

// void ScrollMove(TransformComponent* transformComponent, GLFWwindow* window)
// {
// 	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) return;

// 	m_fovy -= Engine::Context()->windowSystem->scrollOffsetY * m_sensitivity * 2;
// 	m_fovy = m_fovy > 135 ? 135 : m_fovy;
// 	m_fovy = m_fovy < 30 ? 30 : m_fovy;
// }

static void Move(std::shared_ptr<TransformComponent> transformComponent, float deltaTime, GLFWwindow* window)
{
    KeyMove(transformComponent, deltaTime, window);
    MouseMove(transformComponent, deltaTime, window);
}

static void ShowTransform(Transform& transform)
{
    std::cout << transform.Front().transpose() << std::endl;
    std::cout << transform.Up().transpose() << std::endl;
    std::cout << transform.Right().transpose() << std::endl;
    std::cout << transform.GetPosition().transpose() << std::endl;
    std::cout << transform.GetRotation() << std::endl;
    std::cout << transform.GetScale().transpose() << std::endl;
    std::cout << transform.GetMatrix() << std::endl;
    std::cout << transform.GetInverseMatrix() << std::endl;
    std::cout << "////////////////////////////////////" << std::endl;

    Vec3 eulerAngle = transform.GetEulerAngle();
    Quaternion quaternion = transform.GetRotation();

    std::cout << eulerAngle.transpose() << std::endl;
    std::cout << Math::ToEulerAngle(quaternion).transpose() << std::endl;
    std::cout << Math::ToEulerAngle(Math::ToQuaternion(eulerAngle)).transpose() << std::endl;
    std::cout << quaternion << std::endl;
    std::cout << Math::ToQuaternion(eulerAngle) << std::endl;
    std::cout << Math::ToQuaternion(Math::ToEulerAngle(quaternion)) << std::endl;
    std::cout << "////////////////////////////////////" << std::endl;
}

static void TestMath()
{
    Quaternion quaternion = Quaternion::Identity();
    Vec3 angle = Vec3(30.0f, 45.0f, 70.0f);
    std::cout << angle << std::endl;
     
    quaternion = Math::ToQuaternion(angle);
    std::cout << quaternion << std::endl;

    angle = Math::ToEulerAngle(quaternion);
    std::cout << angle << std::endl;

    quaternion = Math::ToQuaternion(angle);
    std::cout << quaternion << std::endl;

    std::cout << "////////////////////////////////////" << std::endl;

    Transform transform = Transform(Vec3(1.0f, 2.0f, 3.0f), Vec3::Ones(), Vec3::Zero());
    ShowTransform(transform);
    transform.SetRotation(Vec3(0, 20, 0));
    ShowTransform(transform);
    transform.Rotate(Vec3(0, 10, 0));
    ShowTransform(transform);
    transform.Rotate(Vec3(0, 10, 0));
    ShowTransform(transform);
}