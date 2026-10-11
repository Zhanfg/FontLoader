# FontLoader 1.2.4 — staged, not released

This branch plans an installer-only 1.2.4 release candidate based on the **exact
uploaded v1.2.3** binary module (not the unrelated v1.1.0 fork master sources).

The original v1.2.3 ZIP has pinned SHA-256:
`d4d52e60e6a06d3027010e8d3c993ed06198b7a9b90898d3ae32d48ace761182`.

After the original file is committed to
`baseline/Font-Loader-v1.2.3-33-b558385-release.zip`, the Action will
preserve its four ELF binaries and generate v1.2.4-rc1, versionCode 34.
Until then CI checks the scripts but does **not** create an installable ZIP.

Changes: architecture detection, installation checksums, legacy-module conflict
guard, `skip_mount`, default `disable`, removal of the dead updateJson URL.
The compiled hooks are unchanged; Android 16/17 boot stability is unverified.
Do not flash this candidate on the device that previously boot-hung.
