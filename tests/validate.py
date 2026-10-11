from pathlib import Path
import ast
p=Path("tools/repack.py").read_text()
ast.parse(p)
assert "versionCode=34" in p
assert "d4d52e60e6a06d3027010e8d3c993ed06198b7a9b90898d3ae32d48ace761182" in p
assert "updateJson=" not in p
install=Path("packaging/customize.sh").read_text()
assert 'touch "$MODPATH/disable"' in install
assert 'touch "$MODPATH/skip_mount"' in install
assert 'font-loader' in install
assert "sha256sum" in Path("packaging/verify.sh").read_text()
print("Preflight checks passed (not a boot test).")
