#include <jni.h>
#include <sys/system_properties.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>
#include "zygisk.hpp"
#include "logging.h"
#include "boot_guard.h"

namespace {
constexpr char kEnable[] = "/data/adb/modules/font-loader/enable-preload";
constexpr char kTarget[] = "/data/adb/modules/font-loader/target.txt";
constexpr char kFont[] = "/data/adb/modules/font-loader/font.txt";

bool ReadOneLine(const char* file, std::string& out) {
    FILE* f = fopen(file, "r");
    if (!f) return false;
    char buffer[4096] {};
    bool ok = fgets(buffer, sizeof(buffer), f) != nullptr;
    fclose(f);
    if (!ok || strlen(buffer) >= sizeof(buffer)-1) return false;
    out = buffer;
    while (!out.empty() && (out.back() == '\n' || out.back() == '\r'))
        out.pop_back();
    return !out.empty();
}
bool BootCompleted() {
    char boot[PROP_VALUE_MAX] {};
    return __system_property_get("sys.boot_completed", boot) > 0
        && strcmp(boot, "1") == 0;
}
bool TargetApp(JNIEnv* env, jstring name) {
    if (!name) return false;
    std::string target;
    if (!ReadOneLine(kTarget, target) || target.size() > 127) return false;
    const char* process = env->GetStringUTFChars(name, nullptr);
    if (!process) {
        if (env->ExceptionCheck()) env->ExceptionClear();
        return false;
    }
    const bool matches = target == process;
    env->ReleaseStringUTFChars(name, process);
    return matches;
}
void MaybeWarm(JNIEnv* env) {
    std::string file;
    if (!ReadOneLine(kFont, file) || !fontloader::SafeFontPath(file)) return;
    struct stat st {};
    if (lstat(file.c_str(), &st) != 0 || !S_ISREG(st.st_mode) ||
        st.st_size <= 0 || st.st_size > 48LL*1024*1024) return;
    jclass klass = env->FindClass("android/graphics/Typeface");
    if (!klass) {
        if (env->ExceptionCheck()) env->ExceptionClear();
        return;
    }
    jmethodID method = env->GetStaticMethodID(klass,
            "nativeWarmUpCache", "(Ljava/lang/String;)V");
    if (!method) {
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(klass);
        return;
    }
    jstring path = env->NewStringUTF(file.c_str());
    if (path) {
        env->CallStaticVoidMethod(klass, method, path);
        env->DeleteLocalRef(path);
    }
    if (env->ExceptionCheck()) {
        LOGW("Warmup failed for selected app");
        env->ExceptionClear();
    }
    env->DeleteLocalRef(klass);
}
class BootGuard final : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        api_ = api; env_ = env;
    }
    void preAppSpecialize(zygisk::AppSpecializeArgs* args) override {
        api_->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        // Never do font work while the OS is booting, or in system processes.
        if (!args || !fontloader::EligibleApp(args->uid) ||
            !BootCompleted() || access(kEnable, F_OK) != 0) return;
        // No companion IPC. Only one explicit font and one exact user app.
        if (TargetApp(env_, args->nice_name)) MaybeWarm(env_);
    }
    void preServerSpecialize(zygisk::ServerSpecializeArgs*) override {
        api_->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
    }
private:
    zygisk::Api* api_ = nullptr;
    JNIEnv* env_ = nullptr;
};
}
REGISTER_ZYGISK_MODULE(BootGuard)
