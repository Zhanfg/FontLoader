#include "font_scan.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_set>

namespace fontloader {
namespace {

constexpr size_t kMaxFonts = 1024;

std::string Join(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    return a.back() == '/' ? a + b : a + "/" + b;
}

bool IsDirectory(const std::string& path) {
    struct stat st {};
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

bool IsRegularFile(const std::string& path) {
    struct stat st {};
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

bool Exists(const std::string& path) {
    return access(path.c_str(), F_OK) == 0;
}

bool IsSafeRelativePath(std::string_view value) {
    if (value.empty() || value.front() == '/' || value.back() == '/') return false;
    // Reject traversal components; do not trust modules' directory entry names.
    size_t start = 0;
    while (start < value.size()) {
        const size_t end = value.find('/', start);
        const std::string_view component =
                value.substr(start, end == std::string_view::npos ? end : end - start);
        if (component.empty() || component == "." || component == "..") return false;
        if (end == std::string_view::npos) break;
        start = end + 1;
    }
    return true;
}

}  // namespace

bool IsSupportedFontFile(std::string_view filename) {
    if (filename.empty() || filename.front() == '.') return false;
    const size_t dot = filename.rfind('.');
    if (dot == std::string_view::npos) return false;
    std::string ext(filename.substr(dot));
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    // TTC/OTC collections may contain multiple faces, including VF faces.
    return ext == ".ttf" || ext == ".otf" || ext == ".ttc" || ext == ".otc";
}

std::vector<std::string> CollectMountedFonts(const ScanOptions& options) {
    std::vector<std::string> fonts;
    std::unordered_set<std::string> seen;
    DIR* modules = opendir(options.modules_root.c_str());
    if (!modules) return fonts;

    while (const dirent* module = readdir(modules)) {
        const std::string name(module->d_name);
        if (!IsSafeRelativePath(name)) continue;
        const std::string module_root = Join(options.modules_root, name);
        if (!IsDirectory(module_root) ||
            Exists(Join(module_root, "disable")) ||
            Exists(Join(module_root, "remove"))) {
            continue;
        }

        for (const auto& mount : options.mounts) {
            if (!IsSafeRelativePath(mount.overlay_relative) ||
                mount.runtime_directory.empty() ||
                mount.runtime_directory.front() != '/') continue;

            const std::string source_dir = Join(module_root, mount.overlay_relative);
            DIR* source = opendir(source_dir.c_str());
            if (!source) continue;

            while (const dirent* file = readdir(source)) {
                const std::string filename(file->d_name);
                if (!IsSupportedFontFile(filename) || filename.find('/') != std::string::npos)
                    continue;
                const std::string source_path = Join(source_dir, filename);
                const std::string runtime_path = Join(mount.runtime_directory, filename);
                if (!IsRegularFile(source_path) || !IsRegularFile(runtime_path)) continue;
                if (seen.emplace(runtime_path).second) fonts.emplace_back(runtime_path);
                if (fonts.size() >= kMaxFonts) break;
            }
            closedir(source);
            if (fonts.size() >= kMaxFonts) break;
        }
        if (fonts.size() >= kMaxFonts) break;
    }
    closedir(modules);
    std::sort(fonts.begin(), fonts.end());
    return fonts;
}

}  // namespace fontloader
