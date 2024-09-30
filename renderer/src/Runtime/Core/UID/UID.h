#pragma once

#include "Core/Serialize/Serializable.h"

//#define UUID_SYSTEM_GENERATOR // 用不了？
#include <uuid.h>

#include <functional>
#include <string>

using namespace uuids;

class UID
{
public:
    UID();
    explicit UID(std::string str);
    UID(const UID& other) = default;
    ~UID() = default;

    static UID Empty();
    inline bool IsEmpty() const                     { return id.is_nil(); }
    inline const std::string& ToString() const      { return str; }

    bool operator== (const UID& other) const
    {
        return this->id == other.id;
    }

private:
    uuid id;
    std::string str;
    static std::mt19937 randomGenerator;

    friend class std::hash<UID>;

private:
    BeginSerailize()
    SerailizeEntry(str) // 读文件的字符串，再转uuid对象
    id = uuids::uuid::from_string(str).value();
    EndSerailize
};

template<>
struct std::hash<UID>
{
    size_t operator()(UID const& uid) const noexcept
    {
        return std::hash<uuid>()(uid.id);
    }
};
