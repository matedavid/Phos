#pragma once

#include "core.h"

#include <filesystem>
#include <optional>

class FileDialog {
  public:
    FileDialog() = delete;

    [[nodiscard]] static std::optional<std::filesystem::path> open_file_dialog(const std::string& file_filter = "");
    [[nodiscard]] static std::vector<std::filesystem::path> open_file_dialog_multiple(
        const std::string& file_filter = "");
};