#include <jni.h>
#include <limits.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "font_scan.h"
#include "logging.h"
#include "misc.h"
#include "zygisk.hpp"

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

namespace {

constexpr int kMaxFonts = 1024;

void PreloadFonts(JNIEnv* env, const std::vector<std::string>& fonts) {
    if (fonts.empty()) return;

    jclass typeface = env->FindClass("android/graphics/Typeface");
    if (!typeface) {
        if (env->ExceptionCheck()) env->ExceptionClear();
        LOGW("Typeface is unavailable; cannot warm font cache");
        return;
    }
    // Skia caches the *font file* here, not a single variable-font weight.
    // This also works for TTC/OTC and OpenType variation axes, which the
    // framework resolves later from the system's font_fallback.xml.
    jmethodID warm_up =
            env->GetStaticMethodID(typeface, "nativeWarmUpCache", "(Ljava/lang/String;)V");
    if (!warm_up) {
        if (env->ExceptionCheck()) env->ExceptionClear();
        LOGW("Typeface.nativeWarmUpCache is unavailable on this ROM");
        env->DeleteLocalRef(typeface);
        return;
    }

    for (const auto& font : fonts) {
        jstring path = env->NewStringUTF(font.c_str());
        if (!path) {
            if (env->ExceptionCheck()) env->ExceptionClear();
            LOGW("Unable to allocate JNI string for font warmup");
            break;
        }
        env->CallStaticVoidMethod(typeface, warm_up, path);
        env->DeleteLocalRef(path);
        if (env->ExceptionCheck()) {
            LOGW("Cannot preload %s", font.c_str());
            env->ExceptionClear();
        }
    }
    env->DeleteLocalRef(typeface);
}

class ZygiskModule final : public zygisk::ModuleBase {
public:
    void onLoad(Api* loaded_api, JNIEnv* loaded_env) override {
        api_ = loaded_api;
        env_ = loaded_env;
    }

    void preAppSpecialize(AppSpecializeArgs* /*args*/) override {
        LoadFontPaths();
        PreloadFonts(env_, fonts_);
        api_->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
    }

    void preServerSpecialize(ServerSpecializeArgs* /*args*/) override {
        api_->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
    }

private:
    Api* api_ = nullptr;
    JNIEnv* env_ = nullptr;
    std::vector<std::string> fonts_;

    void LoadFontPaths() {
        const int companion = api_->connectCompanion();
        if (companion < 0) {
            LOGW("FontLoader companion is unavailable");
            return;
        }

        const int count = read_int(companion);
        if (count < 0 || count > kMaxFonts) {
            LOGW("Invalid font count from companion: %d", count);
            close(companion);
            return;
        }

        char path[PATH_MAX];
        for (int i = 0; i < count; ++i) {
            const int length = read_int(companion);
            if (length <= 0 || length >= static_cast<int>(sizeof(path)) ||
                read_full(companion, path, static_cast<size_t>(length)) != 0) {
                LOGW("Truncated or invalid font path from companion");
                fonts_.clear();
                break;
            }
            path[length] = '\0';
            if (path[0] != '/' ||
                std::string(path, static_cast<size_t>(length)).find('\0') != std::string::npos) {
                LOGW("Invalid font path from companion");
                fonts_.clear();
                break;
            }
            fonts_.emplace_back(path, static_cast<size_t>(length));
        }
        close(companion);
    }
};

void CompanionEntry(int socket) {
    // Zygisk launches a new companion after a reboot; caching within that
    // process avoids scanning every systemless font module for each app fork.
    static const std::vector<std::string> fonts =
            fontloader::CollectMountedFonts(fontloader::ScanOptions{});

    write_int(socket, static_cast<int>(fonts.size()));
    for (const auto& font : fonts) {
        write_int(socket, static_cast<int>(font.size()));
        if (write_full(socket, font.data(), font.size()) != 0) break;
    }
    close(socket);
}

}  // namespace

REGISTER_ZYGISK_MODULE(ZygiskModule)
REGISTER_ZYGISK_COMPANION(CompanionEntry)
