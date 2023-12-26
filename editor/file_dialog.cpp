#include "file_dialog.h"

#include <nfd.h>

#include "utility/logging.h"

std::optional<std::filesystem::path> FileDialog::open_file_dialog(const std::string& file_filter) {
    nfdchar_t* out_path = nullptr;
    const auto result = NFD_OpenDialog(file_filter.c_str(), nullptr, &out_path);

    switch (result) {
    default:
    case NFD_OKAY: {
        auto path = std::filesystem::path(out_path);
        free(out_path);
        return path;
    }
    case NFD_CANCEL:
        free(out_path);
        return {};
    case NFD_ERROR:
        free(out_path);
        PHOS_LOG_ERROR("[FileDialog] Error opening file dialog {}", NFD_GetError());
        return {};
    }
}
std::vector<std::filesystem::path> FileDialog::open_file_dialog_multiple(const std::string& file_filter) {
    nfdpathset_t out_paths;
    const auto result = NFD_OpenDialogMultiple(file_filter.c_str(), nullptr, &out_paths);

    switch (result) {
    default:
    case NFD_OKAY: {
        std::vector<std::filesystem::path> paths(out_paths.count);
        for (std::size_t i = 0; i < NFD_PathSet_GetCount(&out_paths); ++i) {
            const auto path = std::filesystem::path(NFD_PathSet_GetPath(&out_paths, i));
            paths[i] = path;
        }
        NFD_PathSet_Free(&out_paths);

        return paths;
    }
    case NFD_CANCEL:
        NFD_PathSet_Free(&out_paths);
        return {};
    case NFD_ERROR:
        NFD_PathSet_Free(&out_paths);
        PHOS_LOG_ERROR("[FileDialog] Error opening file dialog {}", NFD_GetError());
        return {};
    }
}
