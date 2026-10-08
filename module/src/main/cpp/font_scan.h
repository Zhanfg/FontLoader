#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace fontloader {

// A module contributes <modules_root>/<module>/<overlay_relative>, which is
// exposed to apps at <runtime_directory> when its systemless mount is active.
struct FontMount {
    std::string overlay_relative;
    std::string runtime_directory;
};

struct ScanOptions {
    std::string modules_root = "/data/adb/modules";
    std::vector<FontMount> mounts = {
        {"system/fonts", "/system/fonts"},
        {"product/fonts", "/product/fonts"},
        {"system/product/fonts", "/product/fonts"},
        {"system_ext/fonts", "/system_ext/fonts"},
        {"system/system_ext/fonts", "/system_ext/fonts"},
        {"vendor/fonts", "/vendor/fonts"},
        {"system/vendor/fonts", "/vendor/fonts"},
        {"odm/fonts", "/odm/fonts"},
        {"system/odm/fonts", "/odm/fonts"},
        {"my_product/fonts", "/my_product/fonts"},
        {"system/my_product/fonts", "/my_product/fonts"},
        {"my_stock/fonts", "/my_stock/fonts"},
        {"system/my_stock/fonts", "/my_stock/fonts"},
    };
};

bool IsSupportedFontFile(std::string_view filename);
std::vector<std::string> CollectMountedFonts(const ScanOptions& options);

}  // namespace fontloader
