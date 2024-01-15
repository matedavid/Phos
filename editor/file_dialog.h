#pragma once

#include <filesystem>
#include <optional>
#include <vector>

class FileDialog {
  public:
    FileDialog() = delete;

    [[nodiscard]] static std::optional<std::filesystem::path> open_file_dialog(
        const std::vector<std::pair<std::string, std::string>>& file_filter = {});
    [[nodiscard]] static std::vector<std::filesystem::path> open_file_dialog_multiple(
        const std::vector<std::pair<std::string, std::string>>& file_filter = {});
};
