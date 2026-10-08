#include "../module/src/main/cpp/boot_guard.h"
#include <cassert>
#include <iostream>
int main() {
    using namespace fontloader;
    assert(!EligibleApp(0) && !EligibleApp(1000) && !EligibleApp(99000));
    assert(EligibleApp(10000) && EligibleApp(110001));
    assert(SafeFontPath("/system/fonts/Variable.ttf"));
    assert(SafeFontPath("/product/fonts/Variable.TTC"));
    assert(!SafeFontPath("/system/fonts/../../data/font.ttf"));
    assert(!SafeFontPath("/data/adb/modules/other/font.ttf"));
    assert(!SafeFontPath("/system/fonts/font.woff"));
    std::cout << "PASS boot guard policy and valid font paths\n";
}
