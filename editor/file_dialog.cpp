#include "file_dialog.h"

#include <nfd.h>

#include "utility/logging.h"

std::optional<std::filesystem::path> FileDialog::open_file_dialog(
    const std::vector<std::pair<std::string, std::string>>& file_filter) {
    NFD_Init();

    std::vector<nfdu8filteritem_t> internal_filter_list;
    for (const auto& [name, spec] : file_filter) {
        nfdu8filteritem_t internal_filter;
        internal_filter.name = name.c_str();
        internal_filter.spec = spec.c_str();
        internal_filter_list.push_back(internal_filter);
    }

    nfdchar_t* out_path = nullptr;
    const auto result = NFD_OpenDialog(&out_path, internal_filter_list.data(), internal_filter_list.size(), nullptr);

    switch (result) {
    default:
    case NFD_OKAY: {
        auto path = std::filesystem::path(out_path);
        free(out_path);
        NFD_Quit();
        return path;
    }
    case NFD_CANCEL:
        NFD_Quit();
        return {};
    case NFD_ERROR:
        free(out_path);
        PHOS_LOG_ERROR("[FileDialog] Error opening file dialog {}", NFD_GetError());
        NFD_Quit();
        return {};
    }
}

std::vector<std::filesystem::path> FileDialog::open_file_dialog_multiple(
    const std::vector<std::pair<std::string, std::string>>& file_filter) {
    NFD_Init();

    std::vector<nfdu8filteritem_t> internal_filter_list;
    for (const auto& [name, spec] : file_filter) {
        nfdu8filteritem_t internal_filter;
        internal_filter.name = name.c_str();
        internal_filter.spec = spec.c_str();
        internal_filter_list.push_back(internal_filter);
    }

    const nfdpathset_t* out_paths;
    const auto result =
        NFD_OpenDialogMultiple(&out_paths, internal_filter_list.data(), internal_filter_list.size(), nullptr);

    switch (result) {
    default:
    case NFD_OKAY: {
        nfdpathsetsize_t num_paths;
        NFD_PathSet_GetCount(out_paths, &num_paths);

        std::vector<std::filesystem::path> paths(num_paths);

        for (std::size_t i = 0; i < num_paths; ++i) {
            nfdchar_t* path;
            NFD_PathSet_GetPath(out_paths, i, &path);
            const auto p = std::filesystem::path(path);
            paths[i] = p;

            NFD_PathSet_FreePath(path);
        }

        NFD_PathSet_Free(out_paths);

        NFD_Quit();
        return paths;
    }
    case NFD_CANCEL:
        NFD_PathSet_Free(&out_paths);
        NFD_Quit();
        return {};
    case NFD_ERROR:
        NFD_PathSet_Free(&out_paths);
        PHOS_LOG_ERROR("[FileDialog] Error opening file dialog {}", NFD_GetError());
        NFD_Quit();
        return {};
    }
}
