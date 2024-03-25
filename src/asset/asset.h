#pragma once

#include <string>
#include <optional>
#include "core/uuid.h"

namespace Phos {

class AssetType {
  public:
    enum Value : uint8_t {
        Texture,
        Cubemap,
        Shader,
        Material,
        Mesh,
        Model,
        Prefab,
        Scene,
        Script,
    };

    AssetType() = default;
    constexpr AssetType(Value type) : m_type(type) {}

    constexpr operator Value() const { return m_type; }
    explicit operator bool() const = delete;

    static std::optional<std::string> to_string(const AssetType& type);
    static std::optional<AssetType> from_string(const std::string& str);

  private:
    Value m_type{};
};

class IAsset {
  public:
    virtual ~IAsset() = default;

    virtual AssetType asset_type() = 0;
    UUID id{};
    std::string asset_name;
};

} // namespace Phos
