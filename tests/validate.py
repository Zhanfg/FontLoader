from pathlib import Path
import ast
p=Path("tools/repack.py").read_text()
ast.parse(p)
assert "versionCode=34" in p
assert "d4d52e60e6a06d3027010e8d3c993ed06198b7a9b90898d3ae32d48ace761182" in p
assert "updateJson=" not in p
install=Path("packaging/customize.sh").read_text()
assert 'touch "$MODPATH/disable"' not in install
assert 'rm -f "$MODPATH/disable"' in install
assert "version=v1.2.4-rc2" in p
assert 'touch "$MODPATH/skip_mount"' in install
assert 'font-loader' in install
assert "sha256sum" in Path("packaging/verify.sh").read_text()
print("Preflight OK: default enabled, original 1.2.3 ELF; not a device boot test.")
