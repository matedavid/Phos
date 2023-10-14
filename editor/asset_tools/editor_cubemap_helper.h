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

    ~EditorCubemapHelper() = default;

    [[nodiscard]] static std::shared_ptr<EditorCubemapHelper> create(std::string name);
    [[nodiscard]] static std::shared_ptr<EditorCubemapHelper> open(const std::filesystem::path& path);

    void update_face(Face face, Phos::UUID id);
    void save() const;
    void save(const std::filesystem::path& path) const;

    [[nodiscard]] std::string get_cubemap_name() const { return m_cubemap_name; }
    [[nodiscard]] Faces get_faces() const { return m_faces; }

  private:
    std::string m_cubemap_name;
    std::filesystem::path m_path;
    Phos::UUID m_cubemap_id;

    Faces m_faces;

    explicit EditorCubemapHelper(std::string name);
    explicit EditorCubemapHelper(const std::filesystem::path& path);
};
