# FontLoader (Android 12–17 modernization)

This fork preloads **systemless-mounted font files before app specialization**
to preserve access when a Magisk denylist or other mount namespace boundary
removes the original module's files.

## Compatibility

- Android 12–17 (API 31–37 **runtime** support target; device validation required).
- Scans systemless fonts in `/system/fonts`, `/product/fonts`,
  `/system_ext/fonts`, `/vendor/fonts`, `/odm/fonts`, and optional OPlus
  `/my_product/fonts` and `/my_stock/fonts`.
- Supports `.ttf`, `.otf`, `.ttc`, and `.otc` (including variable fonts).
- Skips disabled/removed font modules. Deduplicates paths and bounds the
  Zygisk companion protocol.
- No root hiding, vendor framework hooks or changes to font weights.
- The build currently compiles with API 36; **this does not mean Android 17
  is device-certified**. The native font warmup path is checked at runtime.

## Variable fonts

FontLoader warms the *whole font file*, not a list of baked `wght` instances.
The Android framework resolves OpenType axes (such as `wght`, `ital`,
`wdth`, `slnt`, and `opsz`) using the font and its font configuration.

**FontLoader does not create a variable family by itself.** The font module
must supply compatible files and system font configuration. On Android 15+,
vendor variable fallback families belong in `/system/etc/font_fallback.xml`
(or the appropriate product/vendor customization); in supported fallback
families, `supportedAxes="wght,ital"` enables the platform to resolve these
axes dynamically. Do not claim support for a font axis not present in its
OpenType `fvar` table. Avoid assuming `fonts.xml` is authoritative on
Android 15–17.

The AOSP runtime has documented `wght`/`ital` support for
`supportedAxes`. Other axes are useful for app-level variation settings,
but are **not** universally selectable by font fallback configuration.
This module preserves file access; it cannot override ColorOS/HyperOS
font managers or fix broken font tables.

## Testing and rollout

CI compiles the Android module and executes host tests for scanner behavior.
These tests **do not** prove boot safety or font rendering on physical
Android 16/17 devices. Test a disposable device/profile before installing
on your daily driver. Keep an uninstall/recovery route available.

Install with Magisk, KernelSU, or APatch. Magisk requires Zygisk enabled;
KernelSU/APatch need a separately installed Zygisk provider such as
ZygiskNext. Installer compatibility does not prove ROM-level compatibility.
Android 12+ only.

For repeatable visual validation, compare glyph/fallback, regular/bold/italic,
variable `wght` values 100/400/700/900, CJK fallback, and apps inside and
outside the denylist. Check `logcat -s FontLoader` for warmup failures.

## Design

1. Scan active systemless font overlays in the Zygisk companion.
2. Return bounded, deduplicated font paths to each app before specialization.
3. Call `Typeface.nativeWarmUpCache(String)` for each path; the framework
   takes responsibility for TTC and variable-font axis resolution.
4. Unload the module library from the child process.

Upstream: JingMatrix/FontLoader (based on RikkaW/FontLoader).

## Downloading from GitHub Actions

The GitHub Actions **FontLoader-release** artifact is an **outer archive**:
extract `font-loader-1.2.0-beta2-release.zip` from it before flashing.
Do not flash the outer `FontLoader-release.zip` as a module: it does not
contain a root-level `module.prop`. Install only the inner module ZIP.
Our CI verifies that the actual installer extracts and validates the native
libraries successfully on both Magisk and KernelSU-style environments.
