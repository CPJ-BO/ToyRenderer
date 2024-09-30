#pragma once

#include "Eigen/Core"
#include "Eigen/Geometry"

#include <cmath>
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

    static Vec3 ClampEulerAngle(Vec3 angle)
    {
        Vec3 ret = angle;
        if(ret.y() > 89.9f) ret.y() = 89.9f;
        if(ret.y() < -89.9f) ret.y() = -89.9f;

        ret.x() = fmod(ret.x(), 360.0f);
        ret.y() = fmod(ret.y(), 360.0f);
        if(ret.x() > 180.0f) ret.x() -= 360.0f;
        if(ret.y() > 180.0f) ret.y() -= 360.0f;

        return ret;
    }

    // static Vec3 ToEulerAngle(const Quaternion& q)
    // {
    //     return ToAngle(q.toRotationMatrix().eulerAngles(1, 2, 0));    // yaw = UnitY轴, pitch = UnitZ轴, roll = UnitX轴
    //                                                                   // 有奇异性问题
    // }

    // https://blog.csdn.net/WillWinston/article/details/125746107
    static Vec3 ToEulerAngle(const Quaternion& q)   // 确保pitch的范围[-PI/2, PI/2]
    {                                               // yaw和roll都是[-PI, PI]
        double angles[3];

        // yaw (y-axis rotation)
        double sinr_cosp = 2 * (q.w() * q.y() -q.x() * q.z());
        double cosr_cosp = 1 - 2 * (q.y() * q.y() + q.z() * q.z());
        angles[0] = std::atan2(sinr_cosp, cosr_cosp);

        // pitch (z-axis rotation)
        double sinp = 2 * (q.w() * q.z() + q.x() * q.y());
        if (std::abs(sinp) >= 1)
            angles[1] = std::copysign(PI / 2, sinp); // use 90 degrees if out of range
        else
            angles[1] = std::asin(sinp);

        // roll (x-axis rotation)
        double siny_cosp = 2 * (q.w() * q.x() - q.y() * q.z());
        double cosy_cosp = 1 - 2 * (q.x() * q.x() + q.z() * q.z());
        angles[2] = std::atan2(siny_cosp, cosy_cosp);     

        angles[0] *= 180 / PI;  // 有一点点精度问题，可以忽略
        angles[1] *= 180 / PI;
        angles[2] *= 180 / PI; 

        return Vec3(angles[0], angles[1], angles[2]);
    }

    static Quaternion ToQuaternion(Vec3 eulerAngle)
    {
        Vec3 radians = ToRadians(eulerAngle) ;  // 右手系，Y向上
        Eigen::AngleAxisf yaw = Eigen::AngleAxisf(radians(0),Vec3::UnitY()); 
        Eigen::AngleAxisf pitch = Eigen::AngleAxisf(radians(1),Vec3::UnitZ());  
        Eigen::AngleAxisf roll = Eigen::AngleAxisf(radians(2),Vec3::UnitX());

        return yaw * pitch * roll;  // 有顺序
    }

    // 下面这一组也是对的，旋转顺序不一样
    // static Vec3 ToEulerAngle(const Quaternion& q)   // 确保pitch的范围[-PI/2, PI/2]
    // {                                               // yaw和roll都是[-PI, PI]
    //     double angles[3];

    //     // roll (x-axis rotation)
    //     double sinr_cosp = 2 * (q.w() * q.x() + q.y() * q.z());
    //     double cosr_cosp = 1 - 2 * (q.x() * q.x() + q.z() * q.z());
    //     angles[2] = std::atan2(sinr_cosp, cosr_cosp);

    //     // pitch (z-axis rotation)
    //     double sinp = 2 * (q.w() * q.z() - q.y() * q.x());
    //     if (std::abs(sinp) >= 1)
    //         angles[1] = std::copysign(PI / 2, sinp); // use 90 degrees if out of range
    //     else
    //         angles[1] = std::asin(sinp);

    //     // yaw (y-axis rotation)
    //     double siny_cosp = 2 * (q.w() * q.y() + q.x() * q.z());
    //     double cosy_cosp = 1 - 2 * (q.y() * q.y() + q.z() * q.z());
    //     angles[0] = std::atan2(siny_cosp, cosy_cosp);     

    //     angles[0] *= 180 / PI;  // 有一点点精度问题，可以忽略
    //     angles[1] *= 180 / PI;
    //     angles[2] *= 180 / PI; 

    //     return Vec3(angles[0], angles[1], angles[2]);
    // }

    // static Quaternion ToQuaternion(Vec3 eulerAngle)
    // {
    //     Vec3 radians = ToRadians(eulerAngle) ;  // 右手系，Y向上
    //     Eigen::AngleAxisf yaw = Eigen::AngleAxisf(radians(0),Vec3::UnitY()); 
    //     Eigen::AngleAxisf pitch = Eigen::AngleAxisf(radians(1),Vec3::UnitZ());  
    //     Eigen::AngleAxisf roll = Eigen::AngleAxisf(radians(2),Vec3::UnitX());

    //     return roll * pitch * yaw;  // 有顺序
    // }

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
        Mat4 mat = Mat4::Identity();
        mat(0, 0) = 2.0f / (right - left);
        mat(1, 1) = 2.0f / (top - bottom);
        mat(2, 2) = - 1.0f / (far - near);
        mat(0, 3) = - (right + left) / (right - left);
        mat(1, 3) = - (top + bottom) / (top - bottom);
        mat(2, 3) = - near / (far - near);
        return mat;
    }












    static inline uint32_t Align(uint32_t value, uint32_t alignment)
    {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    //UE5 MicrosoftPlatformMath.h

    static inline bool IsNaN(float A) { return _isnan(A) != 0; }
    static inline bool IsNaN(double A) { return _isnan(A) != 0; }
    static inline bool IsFinite(float A) { return _finite(A) != 0; }
    static inline bool IsFinite(double A) { return _finite(A) != 0; }

    static inline uint64_t CountLeadingZeros64(uint64_t Value)
    {
        //https://godbolt.org/z/Ejh5G4vPK	
        // return 64 if value if was 0
        unsigned long BitIndex;
        if (!_BitScanReverse64(&BitIndex, Value)) BitIndex = -1;
        return 63 - BitIndex;
    }

    static inline uint64_t CountTrailingZeros64(uint64_t Value)
    {
        // return 64 if Value is 0
        unsigned long BitIndex;	// 0-based, where the LSB is 0 and MSB is 63
        return _BitScanForward64(&BitIndex, Value) ? BitIndex : 64;
    }

    static inline uint32_t CountLeadingZeros(uint32_t Value)
    {
        // return 32 if value is zero
        unsigned long BitIndex;
        _BitScanReverse64(&BitIndex, uint64_t(Value) * 2 + 1);
        return 32 - BitIndex;
    }

    static inline uint64_t CeilLogTwo64(uint64_t Arg)
    {
        // if Arg is 0, change it to 1 so that we return 0
        Arg = Arg ? Arg : 1;
        return 64 - CountLeadingZeros64(Arg - 1);
    }

    static inline uint32_t FloorLog2(uint32_t Value)
    {
        // Use BSR to return the log2 of the integer
        // return 0 if value is 0
        unsigned long BitIndex;
        return _BitScanReverse(&BitIndex, Value) ? BitIndex : 0;
    }
    static inline uint8_t CountLeadingZeros8(uint8_t Value)
    {
        unsigned long BitIndex;
        _BitScanReverse(&BitIndex, uint32_t(Value) * 2 + 1);
        return uint8_t(8 - BitIndex);
    }

    static inline uint32_t CountTrailingZeros(uint32_t Value)
    {
        // return 32 if value was 0
        unsigned long BitIndex;	// 0-based, where the LSB is 0 and MSB is 31
        return _BitScanForward(&BitIndex, Value) ? BitIndex : 32;
    }

    static inline uint32_t CeilLogTwo(uint32_t Arg)
    {
        // if Arg is 0, change it to 1 so that we return 0
        Arg = Arg ? Arg : 1;
        return 32 - CountLeadingZeros(Arg - 1);
    }

    static inline uint32_t RoundUpToPowerOfTwo(uint32_t Arg)
    {
        return 1 << CeilLogTwo(Arg);
    }

    static inline uint64_t RoundUpToPowerOfTwo64(uint64_t Arg)
    {
        return uint64_t(1) << CeilLogTwo64(Arg);
    }

    static inline uint64_t FloorLog2_64(uint64_t Value)
    {
        unsigned long BitIndex;
        return _BitScanReverse64(&BitIndex, Value) ? BitIndex : 0;
    }
}