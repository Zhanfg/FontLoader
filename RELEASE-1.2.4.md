# FontLoader v1.2.4-rc2 — enabled by default

This RC repackages the exact user-supplied v1.2.3 ZIP (SHA-256 pinned in tools/repack.py).
All four native Zygisk shared libraries retain exactly the same bytes as v1.2.3.

- Module ID: `fontloader`; versionCode: 34.
- Installation is enabled by default: no `disable` marker is created.
- The installer removes any stale disable marker in its staging MODPATH.
- `skip_mount` remains: FontLoader should not mount system fonts itself.
- Detects a conflicting active old `font-loader` test module.
- Checks extracted native library SHA-256 and handles device ABI.
- CI runs packaging, archive integrity and real installer smoke tests.

Native code is **not recompiled**: matching v1.2.3 source code is not in the current
fork, whose master sources correspond to v1.1.0. Thus there is no basis to assert
Android 17 boot stability from CI alone. The owner reports the previous ROM boot
issue has been resolved and explicitly requested default-on behavior.

To disable after installation: use the root manager module toggle or create
`/data/adb/modules/fontloader/disable` using an accessible root shell.
