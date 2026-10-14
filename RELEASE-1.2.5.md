# FontLoader 1.2.5 — Android 16/17 and variable-font groundwork

Base branch: `release/1.2.4`. This branch is deliberately NOT merged to
`master`; the previous experimental module caused a real boot hang.

Current implementation: a read-only `fontTools` analyzer for OpenType
`fvar` axis bounds and Android 15+ `font_fallback.xml`
`supportedAxes` values. Supports TTF, OTF, TTC and OTC. Runs in CI with
unit tests, produces a machine-readable JSON result.

The Android platform's fallback `supportedAxes` mechanism only covers
`wght` and `ital`. Other axes (such as `wdth`, `opsz`, `slnt`) can
be inspected here but cannot safely be assumed as universal system fallback
axes. The checker does not rewrite system XML or perform any Android hooks.

Do not call this `v1.2.5` released: the native source for the supplied
v1.2.3 binary still has not been verified, and Android 17 boot stability
has not been demonstrated. Native-level modifications will be gated behind
source identification and device-level logs, followed by reproducible CI.

Planned version when actually releasable: 1.2.5, versionCode 35.
