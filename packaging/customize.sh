SKIPUNZIP=1
[ -n "$BOOTMODE" ] || abort "! Recovery installation unsupported"
[ -f "$ZIPFILE" ] || abort "! Module ZIP not found"
if [ -d /data/adb/modules/font-loader ] && [ ! -f /data/adb/modules/font-loader/disable ]; then
  abort "! Disable old font-loader module to prevent double Zygisk hooks"
fi
case "$ARCH" in
 arm64) ABI="arm64-v8a"; ABI32="armeabi-v7a" ;;
 arm) ABI="armeabi-v7a"; ABI32="" ;;
 x64) ABI="x86_64"; ABI32="x86" ;;
 x86) ABI="x86"; ABI32="" ;;
 *) abort "! Unsupported architecture" ;;
esac
mkdir -p "$MODPATH/zygisk"
unzip -p "$ZIPFILE" verify.sh > "$TMPDIR/fontloader-verify.sh" || abort "! No verify.sh"
. "$TMPDIR/fontloader-verify.sh"
verify_extract "module.prop" "$MODPATH/module.prop"
verify_extract "lib/$ABI/libfontloader.so" "$MODPATH/zygisk/$ABI.so"
if [ -n "$ABI32" ]; then
  AVAILABLE32="$(getprop ro.product.cpu.abilist32 2>/dev/null)"
  [ -z "$AVAILABLE32" ] || verify_extract "lib/$ABI32/libfontloader.so" "$MODPATH/zygisk/$ABI32.so"
fi
touch "$MODPATH/skip_mount" || abort "! Cannot set skip_mount"
touch "$MODPATH/disable" || abort "! Cannot set disable safeguard"
set_perm_recursive "$MODPATH" 0 0 0755 0644
ui_print "- v1.2.4-rc1 installed with ORIGINAL v1.2.3 native code"
ui_print "- SAFE: module disabled by default until reviewed"
