/*
 * Dawn Engine
 * Written by David Avedissian (c) 2012-2015 (avedissian.david@gmail.com)
 */
#include "Common.h"
#include "Camera.h"
#include "Position.h"

NAMESPACE_BEGIN

const Position Position::origin;

Position::Position() : x(0.0), y(0.0), z(0.0)
{
}

Position::Position(double _x, double _y, double _z) : x(_x), y(_y), z(_z)
{
}

Position::Position(const Vec3& vector)
    : x((double)vector.x), y((double)vector.y), z((double)vector.z)
{
}

Position::Position(const Position& other) : x(other.x), y(other.y), z(other.z)
{
}

Vec3 Position::GetRelativeToPoint(const Position& point) const
{
    Position delta = *this - point;
    return Vec3((float)delta.x, (float)delta.y, (float)delta.z);
}

Vec3 Position::ToCameraSpace(Camera* camera) const
{
    return GetRelativeToPoint(camera->GetPosition());
}

Position Position::FromCameraSpace(Camera* camera, const Vec3& cameraSpace)
{
    return camera->GetPosition() + cameraSpace;
}

bool Position::operator==(const Position& other) const
{
    return
        FloatEq(x, other.x) && 
        FloatEq(y, other.y) && 
        FloatEq(z, other.z);
}

bool Position::operator!=(const Position& other) const
{
    return !(*this == other);
}

Position& Position::operator=(const Position& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
}

Position& Position::operator=(const Vec3& other)
{
    return *this = Position(other);
}

Position& Position::operator+=(const Position& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Position& Position::operator+=(const Vec3& other)
{
    return *this += Position(other);
}

Position& Position::operator-=(const Position& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

Position& Position::operator-=(const Vec3& other)
{
    return *this -= Position(other);
}

Position& Position::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

Position& Position::operator/=(float scalar)
{
    return *this *= (1.0f / scalar);
}

const Position Position::operator-() const
{
    return Position(-x, -y, -z);
}

const Position Position::operator+(const Position& other) const
{
    return Position(x + other.x, y + other.y, z + other.z);
}

const Position Position::operator+(const Vec3& other) const
{
    return *this + Position(other);
}

const Position Position::operator-(const Position& other) const
{
    return Position(x - other.x, y - other.y, z - other.z);
}

const Position Position::operator-(const Vec3& other) const
{
    return *this - Position(other);
}

const Position Position::operator*(float scalar) const
{
    return Position(x * scalar, y * scalar, z * scalar);
}

const Position Position::operator/(float scalar) const
{
    return *this * (1.0f / scalar);
}

NAMESPACE_END