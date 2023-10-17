#include "scene_configuration_panel.h"

#include "asset/asset.h"
#include "asset/editor_asset_manager.h"

#include "scene/scene.h"
#include "scene/scene_renderer.h"

#include "renderer/backend/cubemap.h"

SceneConfigurationPanel::SceneConfigurationPanel(std::string name,
                                                 Phos::SceneRendererConfig config,
                                                 std::shared_ptr<Phos::EditorAssetManager> asset_manager)
      : m_name(std::move(name)), m_config(std::move(config)), m_asset_manager(std::move(asset_manager)) {}

#define RENDER_FUNC(func) \
    func();               \
    ImGui::Separator()

void SceneConfigurationPanel::on_imgui_render() {
    ImGui::Begin(m_name.c_str());

    RENDER_FUNC(render_bloom_config);
    RENDER_FUNC(render_environment_config);

    if (ImGui::Button("Update")) {
        m_scene_config_updated_callback(m_config);
    }

    ImGui::End();
}

void SceneConfigurationPanel::render_bloom_config() {
    auto& config = m_config.bloom_config;
    ImGui::Text("Bloom Config");

    ImGui::Checkbox("Enabled", &config.enabled);

    ImGui::AlignTextToFramePadding();

    ImGui::Text("Threshold:");
    ImGui::SameLine();
    ImGui::InputFloat("##BloomConfigThreshold", &config.threshold);
}

void SceneConfigurationPanel::render_environment_config() {
    auto& config = m_config.environment_config;
    ImGui::Text("Environment Config");

    ImGui::AlignTextToFramePadding();

    ImGui::Text("Skybox:");
    ImGui::SameLine();

    auto skybox_name = config.skybox == nullptr ? "" : config.skybox->asset_name;
    ImGui::InputText("##SkyboxConfigInput", skybox_name.data(), skybox_name.size(), ImGuiInputTextFlags_ReadOnly);

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
            const auto uuid = Phos::UUID(*(uint64_t*)payload->Data);

            const auto asset_type = m_asset_manager->get_asset_type(uuid);
            if (asset_type == Phos::AssetType::Cubemap)
                config.skybox = m_asset_manager->load_by_id_type<Phos::Cubemap>(uuid);
        }

        ImGui::EndDragDropTarget();
    }
}
