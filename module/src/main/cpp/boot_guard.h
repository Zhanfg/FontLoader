#pragma once
#include <string_view>
namespace fontloader {
inline bool EligibleApp(int uid) {
    if (uid < 0) return false;
    const int app = uid % 100000;
    return app >= 10000 && app <= 19999;
}
inline bool SafeFontPath(std::string_view s) {
    if (s.empty() || s.front() != '/' || s.back() == '/') return false;
    if (s.find("/../") != s.npos || s.find("/./") != s.npos ||
        s.find("//") != s.npos) return false;
    bool root = false;
    for (auto p : {"/system/fonts/", "/product/fonts/", "/system_ext/fonts/",
                   "/vendor/fonts/", "/odm/fonts/", "/my_product/fonts/",
                   "/my_stock/fonts/"}) {
        if (s.starts_with(p)) {root=true; break;}
    }
    if (!root) return false;
    for (auto e : {".ttf", ".otf", ".ttc", ".otc", ".TTF", ".OTF", ".TTC", ".OTC"}) {
        if (s.ends_with(e)) return true;
    }
    return false;
}
}
