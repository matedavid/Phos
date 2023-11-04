#pragma once

#include "core.h"

#include <imgui.h>
#include <filesystem>
#include <optional>

class ImGuiUtils {
  public:
    template <typename T>
    static inline std::optional<T> drag_drop_target(std::string_view type) {
        auto return_value = std::optional<T>();

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(type.data())) {
                if (payload->DataSize == sizeof(T)) {
                    const auto data = *static_cast<T*>(payload->Data);
                    return_value = data;
                }
            }

            ImGui::EndDragDropTarget();
        }

        return return_value;
    }
};
