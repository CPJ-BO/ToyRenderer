#pragma once

#include "Core/Serialize/Serializable.h"
#include "Math.h"

//坐标系
//world space:      x正向向前，y正向向上，z正向向右
//view space:       x正向向右，y正向向上，z负向向前
//clip space:       x正向向右，y负向向上，z正向向前
//NDC space;        左上[-1, -1]，右下[1, 1]（透视除法后）
//screen space:     左上[0, 0]，右下[1, 1]

class Transform
{
public:
    Transform() = default;
    Transform(Mat4 matrix);
    Transform(Vec3 position, Vec3 scale, Quaternion rotation = Quaternion::Identity()) 
    : position(position)
    , scale(scale)
    , rotation(rotation)
    , eulerAngle(Math::ToEulerAngle(rotation))
    {
        UpdateVector();
    }

    Transform(Vec3 position, Vec3 scale, Vec3 eulerAngle = Vec3::Zero()) 
    : position(position)
    , scale(scale)
    , eulerAngle(eulerAngle)
    , rotation(Math::ToQuaternion(eulerAngle))
    {
        UpdateVector();
    }

    inline Vec3 GetPosition() const         { return position; }
    inline Vec3 GetScale() const            { return scale; }
    inline Quaternion GetRotation() const   { return rotation; }
    inline Vec3 GetEulerAngle() const       { return eulerAngle; }

    inline Vec3 Front() const               { return front; }
    inline Vec3 Up() const                  { return up; }
    inline Vec3 Right() const               { return right; }

    void SetPosition(Vec3 position)         { this->position = position; } 
    void SetScale(Vec3 scale)               { this->scale = scale; } 
    void SetRotation(Quaternion rotation);
    void SetRotation(Vec3 eulerAngle);

    Vec3 Translate(Vec3 translation);
    Vec3 Scale(Vec3 scale);
    Vec3 Rotate(Vec3 angle);
    //Quaternion Rotate(Quaternion rotation);

    Mat4 GetMatrix() const;
    Mat4 GetInverseMatrix() const;

    //Transform RelativeTo(const Transform& other) const;

    Quaternion InverseRotation() const  { return rotation.conjugate(); }
    Vec3 InverseScale() const           { return Vec3(1.0 / scale.x(), 1.0 / scale.y(), 1.0 / scale.z()); }           
    Vec3 InversePosition() const        { return -position; }
    Transform Inverse() const           { return Transform(InversePosition(), InverseScale(), InverseRotation());}

    Transform operator*(const Transform& other) const { return Transform(this->GetMatrix() * other.GetMatrix()); }

private:
    Vec3 position           = Vec3::Zero();
    Vec3 scale              = Vec3::Ones();
    Quaternion rotation     = Quaternion::Identity(); 
    Vec3 eulerAngle         = Vec3::Zero();             // ZYX顺序的欧拉角，角度制  
                                                        // yaw pitch roll 偏航角 俯仰角 滚转角       

    Vec3 front              = Vec3::UnitX();
    Vec3 up                 = Vec3::UnitY();
    Vec3 right              = Vec3::UnitZ();

    void UpdateVector();

private:
    BeginSerailize()
    SerailizeEntry(position)
    SerailizeEntry(scale)
    SerailizeEntry(rotation)
    SetRotation(rotation);   
    EndSerailize
};