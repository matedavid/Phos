#pragma once

#include <filesystem>

#include "core/uuid.h"
#include "asset/asset.h"

struct EditorAsset {
    bool is_directory = false;
    Phos::AssetType type;
    std::filesystem::path path;
    Phos::UUID uuid;
};