#include "../module/src/main/cpp/font_scan.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

namespace fs = std::filesystem;
using fontloader::CollectMountedFonts;
using fontloader::FontMount;
using fontloader::ScanOptions;

static void Touch(const fs::path& path) {
    fs::create_directories(path.parent_path());
    std::ofstream(path) << "test";
}

int main() {
    const auto root = fs::temp_directory_path() /
                      ("fontloader-test-" + std::to_string(getpid()));
    fs::remove_all(root);
    const auto modules = root / "modules";
    const auto system = root / "system/fonts";
    const auto product = root / "product/fonts";
    fs::create_directories(modules);
    Touch(modules / "enabled/system/fonts/Variable.TTF");
    Touch(modules / "enabled/system/fonts/Collection.ttc");
    Touch(modules / "enabled/system/fonts/ignored.woff");
    Touch(modules / "enabled/product/fonts/Serif.otc");
    Touch(modules / "disabled/system/fonts/nope.ttf");
    Touch(modules / "disabled/disable");
    Touch(modules / "removed/system/fonts/nope2.ttf");
    Touch(modules / "removed/remove");
    Touch(modules / "enabled/vendor/fonts/not-mounted.otf");
    Touch(system / "Variable.TTF");
    Touch(system / "Collection.ttc");
    Touch(system / "ignored.woff");
    Touch(product / "Serif.otc");

    ScanOptions options;
    options.modules_root = modules.string();
    options.mounts = {
        {"system/fonts", system.string()},
        {"product/fonts", product.string()},
        {"system/fonts", system.string()},  // no duplicates
        {"../invalid", system.string()},  // no path traversal
    };
    const auto files = CollectMountedFonts(options);
    assert(files.size() == 3);
    assert(files[0] == (system / "Collection.ttc").string() ||
           files[0] == (product / "Serif.otc").string());
    assert(fontloader::IsSupportedFontFile("A.otf"));
    assert(fontloader::IsSupportedFontFile("Font.TTC"));
    assert(fontloader::IsSupportedFontFile("Font.otc"));
    assert(!fontloader::IsSupportedFontFile(".hidden.ttf"));
    assert(!fontloader::IsSupportedFontFile("Font.woff2"));

    fs::remove_all(root);
    std::cout << "Font scan tests passed (multi-partition, VF/collections, dedup, disabled, removed).\n";
}
