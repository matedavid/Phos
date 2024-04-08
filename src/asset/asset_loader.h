#pragma once

#include <filesystem>
#include <vector>

#include "asset/asset.h"

// Forward declarations
namespace YAML {
class Node;
}

namespace Phos {

// Forward declarations
class IAssetParser;
class EditorAssetManager;
class Texture;
class ModelAsset;

class AssetLoader {
  public:
    explicit AssetLoader(EditorAssetManager* manager);
    ~AssetLoader() = default;

    [[nodiscard]] UUID get_id(const std::string& path) const;
    [[nodiscard]] AssetType get_type(const std::string& path) const;
    [[nodiscard]] std::shared_ptr<IAsset> load(const std::string& path) const;

  private:
    std::vector<std::unique_ptr<IAssetParser>> m_parsers;
    EditorAssetManager* m_manager;
};

//
// Parsers
//

class IAssetParser {
  public:
    virtual ~IAssetParser() = default;

    virtual AssetType type() = 0;
    virtual std::shared_ptr<IAsset> parse(const YAML::Node& node, [[maybe_unused]] const std::string& path) = 0;
};

class TextureParser : public IAssetParser {
  public:
    explicit TextureParser([[maybe_unused]] EditorAssetManager*) {}

    [[nodiscard]] AssetType type() override { return AssetType::Texture; }
    std::shared_ptr<IAsset> parse(const YAML::Node& node, const std::string& path) override;
};

class CubemapParser : public IAssetParser {
  public:
    explicit CubemapParser(EditorAssetManager* manager) : m_manager(manager) {}

    [[nodiscard]] AssetType type() override { return AssetType::Cubemap; }
    std::shared_ptr<IAsset> parse(const YAML::Node& node, const std::string& path) override;

  private:
    EditorAssetManager* m_manager;

    [[nodiscard]] std::filesystem::path load_face(const Phos::UUID& id);
};

class MaterialParser : public IAssetParser {
  public:
    explicit MaterialParser(EditorAssetManager* manager) : m_manager(manager) {}

    [[nodiscard]] AssetType type() override { return AssetType::Material; }
    std::shared_ptr<IAsset> parse(const YAML::Node& node, const std::string& path) override;

  private:
    EditorAssetManager* m_manager;

    [[nodiscard]] std::shared_ptr<Texture> parse_texture(const YAML::Node& node) const;
};

class StaticMeshParser : public IAssetParser {
  public:
    explicit StaticMeshParser([[maybe_unused]] EditorAssetManager*) {}

    [[nodiscard]] AssetType type() override { return AssetType::StaticMesh; }
    std::shared_ptr<IAsset> parse(const YAML::Node& node, const std::string& path) override;
};

class PrefabParser : public IAssetParser {
  public:
    explicit PrefabParser(EditorAssetManager* manager) : m_manager(manager) {}

    [[nodiscard]] AssetType type() override { return AssetType::Prefab; }
    std::shared_ptr<IAsset> parse(const YAML::Node& node, const std::string& path) override;

  private:
    EditorAssetManager* m_manager;
};

class SceneParser : public IAssetParser {
  public:
    explicit SceneParser(EditorAssetManager* manager) : m_manager(manager) {}

    [[nodiscard]] AssetType type() override { return AssetType::Scene; }
    std::shared_ptr<IAsset> parse(const YAML::Node& node, const std::string& path) override;

  private:
    EditorAssetManager* m_manager;
};

class ScriptParser : public IAssetParser {
  public:
    explicit ScriptParser(EditorAssetManager* manager) : m_manager(manager) {}

    [[nodiscard]] AssetType type() override { return AssetType::Script; }
    std::shared_ptr<IAsset> parse(const YAML::Node& node, const std::string& path) override;

  private:
    EditorAssetManager* m_manager;
};

} // namespace Phos
