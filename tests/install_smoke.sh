#!/bin/sh
# Execute the *real* module installer on an isolated host fixture.
# Both KSU and Magisk variables are exercised. No system files are changed.
set -eu
ZIP="${1:?Usage: tests/install_smoke.sh path-to-inner-module.zip}"
test -r "$ZIP"
ZIP="$(realpath "$ZIP")"
unzip -tq "$ZIP" >/dev/null
for mode in ksu magisk; do
  work="$(mktemp -d)"
  mkdir -p "$work/tmp" "$work/module"
  unzip -p "$ZIP" customize.sh > "$work/customize.sh"
  (
    export ZIPFILE="$ZIP" TMPDIR="$work/tmp" MODPATH="$work/module"
    export ARCH="arm64" IS64BIT=true BOOTMODE=true API=36
    if [ "$mode" = ksu ]; then
      export KSU=true APATCH=false
    else
      export KSU=false APATCH=false MAGISK_VER_CODE=27000 MAGISK_VER=27.0
    fi
    # The manager normally injects these functions; replace only manager I/O.
    ui_print() { printf '%s\n' "$*"; }
    abort() { printf 'ABORT: %s\n' "$*" >&2; exit 1; }
    set_perm_recursive() { :; }
    . "$work/customize.sh"
  ) > "$work/install.log" 2>&1 || {
    cat "$work/install.log" >&2
    rm -rf "$work"
    exit 1
  }
  for f in "zygisk/arm64-v8a.so" "zygisk/armeabi-v7a.so" "module.prop" "post-fs-data.sh"; do
    test -s "$work/module/$f" || { cat "$work/install.log" >&2; echo "Missing installed $f" >&2; exit 1; }
  done
  grep -q 'Verified lib/arm64-v8a/libfontloader.so' "$work/install.log"
  grep -q 'Verified lib/armeabi-v7a/libfontloader.so' "$work/install.log"
  printf '%s\n' "Installer smoke test passed for $mode"
  rm -rf "$work"
done
