#!/usr/bin/env python3
import hashlib, sys, zipfile
from pathlib import Path

SOURCE_SHA = "d4d52e60e6a06d3027010e8d3c993ed06198b7a9b90898d3ae32d48ace761182"
LIB_HASHES = {
 "lib/arm64-v8a/libfontloader.so": "d2c88dcf0da2d0fce190bff2d4138575a6a3529114e6180288d44b3d5b293e2b",
 "lib/armeabi-v7a/libfontloader.so": "87f37cf32dd0185e222cdb945b0a348915815e3c5657fd7c1924327fd73cee33",
 "lib/x86/libfontloader.so": "29a357bfe158fbc5dd03ca9f16737c5bd4a85f94279d87827b015fba0c99d59a",
 "lib/x86_64/libfontloader.so": "c09adb899008b9b8e451c7bfd2f5850c0fa884edb48bf237730da8945ba67ea1",
}
def sha(data): return hashlib.sha256(data).hexdigest()
def build(input_path, output_path):
    raw = Path(input_path).read_bytes()
    if sha(raw) != SOURCE_SHA: raise ValueError("Original v1.2.3 ZIP hash mismatch")
    with zipfile.ZipFile(input_path) as src:
        names = src.namelist()
        if len(names)!=len(set(names)): raise ValueError("Duplicate ZIP entries")
        if any(n.startswith("/") or ".." in n.split("/") for n in names):
            raise ValueError("Unsafe ZIP path")
        files = {n:src.read(n) for n in names if not n.endswith("/") and not n.endswith(".sha256")}
    if b"id=fontloader\n" not in files["module.prop"] or b"v1.2.3 (33-b558385-release)" not in files["module.prop"]:
        raise ValueError("Incorrect native baseline")
    for name,digest in LIB_HASHES.items():
        if sha(files[name]) != digest: raise ValueError("ELF mismatch: " + name)
    files["module.prop"] = (
        "id=fontloader\nname=Font Loader\nversion=v1.2.4-rc2\nversionCode=34\n"
        "author=aviraxp, RikkaW; guarded packaging by Axymorrsen\n"
        "description=Original v1.2.3 native loader. Enabled by default; guarded installer.\n").encode()
    folder=Path(__file__).resolve().parents[1]
    files["customize.sh"]=(folder/"packaging/customize.sh").read_bytes()
    files["verify.sh"]=(folder/"packaging/verify.sh").read_bytes()
    files["GUARD-NOTICE.txt"]=b"v1.2.4-rc2 preserves all native v1.2.3 ELF bytes. Enabled by default as requested. Android 16/17 runtime not certified.\n"
    Path(output_path).parent.mkdir(parents=True,exist_ok=True)
    with zipfile.ZipFile(output_path,"w",compression=zipfile.ZIP_DEFLATED,compresslevel=9) as z:
        for name,data in sorted(files.items()):
            z.writestr(name,data)
            z.writestr(name+".sha256",sha(data).encode("ascii"))
    print("Guarded v1.2.4 candidate created: " + str(output_path))
if __name__=="__main__":
    if len(sys.argv)!=3: raise SystemExit("Usage: python3 tools/repack.py BASE_ZIP OUT_ZIP")
    build(sys.argv[1],sys.argv[2])
