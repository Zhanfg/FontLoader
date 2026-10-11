#!/bin/sh
# Host simulation of the real root module installer. Writes only to temporary paths.
set -eu
ZIP=$(realpath "$1")
for spec in "arm64:armeabi-v7a:arm64-v8a" "arm::armeabi-v7a" "x64:x86:x86_64" "x86::x86"; do
  ARCH=${spec%%:*}
  tailpart=${spec#*:}
  SECONDARY=${tailpart%%:*}
  PRIMARY=${tailpart#*:}
  WORK=$(mktemp -d)
  mkdir -p "$WORK/temp" "$WORK/module"
  unzip -p "$ZIP" customize.sh > "$WORK/temp/customize.sh"
  (
    export ZIPFILE="$ZIP" TMPDIR="$WORK/temp" MODPATH="$WORK/module"
    export BOOTMODE=true ARCH
    ui_print() { printf "%s\n" "$*"; }
    abort() { echo "ERROR: $*" >&2; exit 1; }
    getprop() {
      if [ "$1" = "ro.product.cpu.abilist32" ] && [ -n "$SECONDARY" ]; then
        echo "$SECONDARY"
      else
        echo ""
      fi
    }
    set_perm_recursive() { :; }
    . "$WORK/temp/customize.sh"
  ) > "$WORK/log" 2>&1 || {
    cat "$WORK/log" >&2
    rm -rf "$WORK"
    exit 1
  }
  test -s "$WORK/module/module.prop"
  test -s "$WORK/module/zygisk/$PRIMARY.so"
  test -f "$WORK/module/skip_mount"
  test ! -e "$WORK/module/disable"
  if [ -n "$SECONDARY" ]; then
    test -s "$WORK/module/zygisk/$SECONDARY.so"
  fi
  grep -q "ENABLED by default" "$WORK/log" || {
    cat "$WORK/log" >&2
    echo "Did not explicitly enable by default" >&2
    exit 1
  }
  echo "Installer smoke test passed: $ARCH"
  rm -rf "$WORK"
done
