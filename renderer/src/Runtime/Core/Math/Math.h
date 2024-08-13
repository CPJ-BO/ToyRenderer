#pragma once

#include "Eigen/Core"
#include "Eigen/Geometry"

#include <cstdint>

#define PI 3.14159265358979323846

// 注意:    Eigen的矩阵是行优先 row
//          glm的矩阵是列优先 column

typedef Eigen::Matrix2f Mat2;
typedef Eigen::Matrix3f Mat3;
typedef Eigen::Matrix4f Mat4;

typedef Eigen::Vector2f Vec2;
typedef Eigen::Vector3f Vec3;
typedef Eigen::Vector4f Vec4;

typedef Eigen::Vector2i IVec2;
typedef Eigen::Vector3i IVec3;
typedef Eigen::Vector4i IVec4;

typedef Eigen::Vector2<uint32_t> UVec2;
typedef Eigen::Vector3<uint32_t> UVec3;
typedef Eigen::Vector4<uint32_t> UVec4;

typedef Eigen::Quaternion<float> Quaternion;

namespace Math 
{
    static inline float ToRadians(float angle) { return angle * PI / 180.0f; }
    static inline Vec2 ToRadians(Vec2 angle) { return angle * PI / 180.0f; }
    static inline Vec3 ToRadians(Vec3 angle) { return angle * PI / 180.0f; }
    static inline Vec4 ToRadians(Vec4 angle) { return angle * PI / 180.0f; }

    static inline float ToAngle(float radians) { return radians * 180.0f / PI; }
    static inline Vec2 ToAngle(Vec2 radians) { return radians * 180.0f / PI; }
    static inline Vec3 ToAngle(Vec3 radians) { return radians * 180.0f / PI; }
    static inline Vec4 ToAngle(Vec4 radians) { return radians * 180.0f / PI; }

    // static Vec3 ToEulerAngle(const Quaternion& q)
    // {
    //     float yaw, pitch, roll;

    //     // yaw (z-axis rotation)
    //     float siny_cosp = +2.0 * (q.w() * q.y() + q.x() * q.z());
    //     float cosy_cosp = +1.0 - 2.0 * (q.z() * q.z() + q.y() * q.y());
    //     yaw = atan2(siny_cosp, cosy_cosp);

    //     // pitch (y-axis rotation)
    //     float sinp = +2.0 * (q.w() * q.z() - q.y() * q.x());
    //     if (fabs(sinp) >= 1)    pitch = copysign(PI / 2, sinp); // use 90 degrees if out of range
    //     else                        pitch = asin(sinp);
        
    //     // roll (x-axis rotation)
    //     float sinr_cosp = +2.0 * (q.w() * q.x() + q.z() * q.y());
    //     float cosr_cosp = +1.0 - 2.0 * (q.x() * q.x() + q.z() * q.z());
    //     roll = atan2(sinr_cosp, cosr_cosp);

    //     return ToAngle(Vec3(yaw, pitch, roll));
    // }

    static Vec3 ToEulerAngle(const Quaternion& q)
    {
        return ToAngle(q.toRotationMatrix().eulerAngles(1, 2, 0));    // yaw = UnitY轴, pitch = UnitZ轴, roll = UnitX轴
    }

    static Quaternion ToQuaternion(Vec3 eulerAngle)
    {
        Vec3 radians = ToRadians(eulerAngle) ;  // 右手系，Y向上
        Eigen::AngleAxisf yaw = Eigen::AngleAxisf(radians(0),Vec3::UnitY()); 
        Eigen::AngleAxisf pitch = Eigen::AngleAxisf(radians(1),Vec3::UnitZ());  
        Eigen::AngleAxisf roll = Eigen::AngleAxisf(radians(2),Vec3::UnitX());

        return yaw * pitch * roll;  // 有顺序
    }

    static Mat4 LookAt(Vec3 eye, Vec3 center, Vec3 up)
    {
        Vec3 f = (center - eye).normalized();
        Vec3 s = f.cross(up).normalized();
        Vec3 u = s.cross(f);

        Mat4 mat = Mat4::Identity();
        mat(0, 0) = s.x();
        mat(0, 1) = s.y();
        mat(0, 2) = s.z();
        mat(1, 0) = u.x();
        mat(1, 1) = u.y();
        mat(1, 2) = u.z();
        mat(2, 0) =-f.x();
        mat(2, 1) =-f.y();
        mat(2, 2) =-f.z();
        mat(0, 3) =-s.dot(eye);
        mat(1, 3) =-u.dot(eye);
        mat(2, 3) = f.dot(eye);

        return mat;
    }

    static Mat4 Perspective(float fovy, float aspect, float near, float far)
    {
        float const tanHalfFovy = tan(fovy / 2.0f);

        Mat4 mat = Mat4::Zero();
        mat(0, 0) = 1.0f / (aspect * tanHalfFovy);
        mat(1, 1) = 1.0f / (tanHalfFovy);
        mat(2, 2) = far / (near - far);
        mat(3, 2) = - 1.0f;
        mat(2, 3) = -(far * near) / (far - near);
        return mat;
    }

    static Mat4 Ortho(float left, float right, float bottom, float top, float near, float far)
    {
        Mat4 mat = Mat4::Ones();
        mat(0, 0) = 2.0f / (right - left);
        mat(1, 1) = 2.0f / (top - bottom);
        mat(2, 2) = - 1.0f / (far - near);
        mat(0, 3) = - (right + left) / (right - left);
        mat(1, 3) = - (top + bottom) / (top - bottom);
        mat(2, 3) = - near / (far - near);
        return mat;
    }
}