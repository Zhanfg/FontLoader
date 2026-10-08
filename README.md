# FontLoader — Boot Guard (experimental, not device-tested)

**Do not flash this version onto the previously affected phone until
the boot hang is recovered and diagnostic logs are reviewed.**

Older beta2 broad-scanned fonts in every app fork and was reported to hang
at the second boot screen. The old PR has been closed. This branch is built
from the upstream master baseline, not beta2.

## Fail-closed runtime

- This Zygisk module does **not mount fonts**. Font overlays belong to
  the user's existing font module and root manager.
- The installer creates skip_mount. It never creates enable-preload.
- Early-boot apps, system_server, SystemUI and isolated processes cannot
  trigger font cache warming.
- No module directory traversal or companion IPC is performed.
- An explicit application and ONE approved font file must be configured.
- Android 16 / 17 runtime compatibility is experimental; not yet validated.

## Enabling one-app diagnosis (ONLY on recovered/test devices)

To opt in after boot, with a *verified* app and font path:

    printf '%s\n' 'com.example.testapp' > /data/adb/modules/font-loader/target.txt
    printf '%s\n' '/system/fonts/YourTestFont.ttf' > /data/adb/modules/font-loader/font.txt
    touch /data/adb/modules/font-loader/enable-preload

This is not enabled by default. It is disabled immediately for future app
launches by removing enable-preload.

## Recovery

Module ID: font-loader

    touch /data/adb/modules/font-loader/disable

KernelSU supports safe mode via rapid repeated Volume Down press/release
after first boot screen, if compiled in the device kernel. Do not wipe
user data or remove unrelated font modules to isolate this issue.

## Variable fonts

.ttf, .otf, .ttc, .otc are eligible if Android has a valid mounted font
file. The module preserves access to a single explicitly configured file;
it does not implement or force fvar, axes, fallback weights or font-family
mappings. Runtime font behavior needs real-device testing.
