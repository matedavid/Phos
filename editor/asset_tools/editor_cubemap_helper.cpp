#include "editor_cubemap_helper.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

std::shared_ptr<EditorCubemapHelper> EditorCubemapHelper::create(std::string name) {
    return std::shared_ptr<EditorCubemapHelper>(new EditorCubemapHelper(std::move(name)));
}

// create constructor
EditorCubemapHelper::EditorCubemapHelper(std::string name) : m_cubemap_name(std::move(name)) {
    m_type = Type::Faces;

    m_faces = {};
    m_equirectangular_id = Phos::UUID(0);
}

std::shared_ptr<EditorCubemapHelper> EditorCubemapHelper::open(const std::filesystem::path& path) {
    return std::shared_ptr<EditorCubemapHelper>(new EditorCubemapHelper(path));
}

#define LOAD_FACE(face) m_faces.face = Phos::UUID(faces_node[#face].as<uint64_t>())

// open constructor
EditorCubemapHelper::EditorCubemapHelper(const std::filesystem::path& path) : m_path(path) {
    const auto node = YAML::LoadFile(path);

    const auto asset_type = node["assetType"].as<std::string>();
    PS_ASSERT(asset_type == "cubemap", "Asset in EditorCubemapHelper is not cubemap ({})", asset_type)

    m_cubemap_id = Phos::UUID(node["id"].as<uint64_t>());
    m_cubemap_name = path.stem();

    const auto cubemap_type = node["type"].as<std::string>();
    if (cubemap_type == "faces") {
        m_type = Type::Faces;

        const auto& faces_node = node["faces"];

        m_faces = {};
        LOAD_FACE(left);
        LOAD_FACE(right);
        LOAD_FACE(top);
        LOAD_FACE(bottom);
        LOAD_FACE(front);
        LOAD_FACE(back);
    } else if (cubemap_type == "equirectangular") {
        m_type = Type::Equirectangular;
        m_equirectangular_id = Phos::UUID(node["texture"].as<uint64_t>());
    } else {
        PS_FAIL("Cubemap type '{}' is not recognized", cubemap_type)
    }
}

void EditorCubemapHelper::update_face(Face face, Phos::UUID id) {
    switch (face) {
    case Face::Left:
        m_faces.left = id;
        break;
    case Face::Right:
        m_faces.right = id;
        break;
    case Face::Top:
        m_faces.top = id;
        break;
    case Face::Bottom:
        m_faces.bottom = id;
        break;
    case Face::Front:
        m_faces.front = id;
        break;
    case Face::Back:
        m_faces.back = id;
        break;
    }
}

void EditorCubemapHelper::update_equirectangular_id(Phos::UUID id) {
    m_equirectangular_id = id;
}

void EditorCubemapHelper::change_type(Type type) {
    m_type = type;
}

void EditorCubemapHelper::save() const {
    if (!std::filesystem::exists(m_path)) {
        PS_ERROR("Could not save cubemap {} because path is not set", m_cubemap_name);
        return;
    }

    save(m_path);
}

void EditorCubemapHelper::save(const std::filesystem::path& path) const {
    YAML::Emitter out;

    out << YAML::BeginMap;

    out << YAML::Key << "assetType" << YAML::Value << "cubemap";
    out << YAML::Key << "id" << YAML::Value << (uint64_t)m_cubemap_id;

    switch (m_type) {
    case Type::Faces: {
        out << YAML::Key << "type" << YAML::Value << "faces";

        out << YAML::Key << "faces" << YAML::BeginMap;

        out << YAML::Key << "left" << YAML::Value << (uint64_t)m_faces.left;
        out << YAML::Key << "right" << YAML::Value << (uint64_t)m_faces.right;
        out << YAML::Key << "top" << YAML::Value << (uint64_t)m_faces.top;
        out << YAML::Key << "bottom" << YAML::Value << (uint64_t)m_faces.bottom;
        out << YAML::Key << "front" << YAML::Value << (uint64_t)m_faces.front;
        out << YAML::Key << "back" << YAML::Value << (uint64_t)m_faces.back;

        out << YAML::EndMap;
    } break;
    case Type::Equirectangular: {
        out << YAML::Key << "type" << YAML::Value << "equirectangular";
        out << YAML::Key << "texture" << YAML::Value << (uint64_t)m_equirectangular_id;
    } break;
    }

    out << YAML::EndMap;

    std::ofstream output_file(path);
    output_file << out.c_str();
}
