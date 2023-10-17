#pragma once

#include "core.h"
#include <filesystem>

#include "core/uuid.h"
#include "renderer/backend/cubemap.h"

class EditorCubemapHelper {
  public:
    struct Faces {
        Phos::UUID left{0};
        Phos::UUID right{0};
        Phos::UUID top{0};
        Phos::UUID bottom{0};
        Phos::UUID front{0};
        Phos::UUID back{0};
    };

    enum class Face {
        Left,
        Right,
        Top,
        Bottom,
        Front,
        Back,
    };

    enum class Type {
        Faces,
        Equirectangular,
    };

    ~EditorCubemapHelper() = default;

    [[nodiscard]] static std::shared_ptr<EditorCubemapHelper> create(std::string name);
    [[nodiscard]] static std::shared_ptr<EditorCubemapHelper> open(const std::filesystem::path& path);

    void update_face(Face face, Phos::UUID id);
    void update_equirectangular_id(Phos::UUID id);

    void change_type(Type type);

    void save() const;
    void save(const std::filesystem::path& path) const;

    [[nodiscard]] std::string get_cubemap_name() const { return m_cubemap_name; }
    [[nodiscard]] Type get_cubemap_type() const { return m_type; }

    [[nodiscard]] Faces get_faces() const { return m_faces; }
    [[nodiscard]] Phos::UUID get_equirectangular_id() const { return m_equirectangular_id; }

  private:
    std::string m_cubemap_name;
    std::filesystem::path m_path;
    Phos::UUID m_cubemap_id;

    Type m_type;

    Faces m_faces;
    Phos::UUID m_equirectangular_id = Phos::UUID(0);

    explicit EditorCubemapHelper(std::string name);
    explicit EditorCubemapHelper(const std::filesystem::path& path);
};
