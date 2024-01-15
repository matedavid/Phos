#include "editor_cubemap_helper.h"

#include <fstream>

#include "utility/logging.h"
#include "asset_tools/asset_builder.h"

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
    PHOS_ASSERT(asset_type == "cubemap", "Asset in EditorCubemapHelper is not cubemap ({})", asset_type);

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
        PHOS_FAIL("Cubemap type '{}' is not recognized", cubemap_type);
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
        PHOS_LOG_ERROR("Could not save cubemap {} because path is not set", m_cubemap_name);
        return;
    }

    save(m_path);
}

void EditorCubemapHelper::save(const std::filesystem::path& path) const {
    auto cubemap_builder = AssetBuilder();

    cubemap_builder.dump("assetType", "cubemap");
    cubemap_builder.dump("id", m_cubemap_id);

    switch (m_type) {
    case Type::Faces: {
        cubemap_builder.dump("type", "faces");

        auto faces_builder = AssetBuilder();

        faces_builder.dump("left", m_faces.left);
        faces_builder.dump("right", m_faces.right);
        faces_builder.dump("top", m_faces.top);
        faces_builder.dump("bottom", m_faces.bottom);
        faces_builder.dump("front", m_faces.front);
        faces_builder.dump("back", m_faces.back);

        cubemap_builder.dump("faces", faces_builder);
    } break;
    case Type::Equirectangular: {
        cubemap_builder.dump("type", "equirectangular");
        cubemap_builder.dump("texture", m_equirectangular_id);
    } break;
    }

    std::ofstream output_file(path);
    output_file << cubemap_builder;
}
