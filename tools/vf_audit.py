#!/usr/bin/env python3
"""Read-only OpenType variable-font and Android fallback configuration audit.

This intentionally does not patch /system, generate systemless overlays, or
hook Android internals. It is safe to run as a preflight in CI.
"""
import argparse
import json
import math
from pathlib import Path
import xml.etree.ElementTree as ET
from fontTools.ttLib import TTFont, TTCollection

def validate_axes(axes):
    errors=[]
    result=[]
    seen=set()
    for axis in axes:
        tag=str(axis.axisTag)
        minimum=float(axis.minValue)
        default=float(axis.defaultValue)
        maximum=float(axis.maxValue)
        if tag in seen: errors.append("duplicate axis tag: "+tag)
        seen.add(tag)
        if len(tag)!=4: errors.append("invalid axis tag length: "+tag)
        if not all(map(math.isfinite,(minimum,default,maximum))):
            errors.append("non-finite coordinate for "+tag)
        elif not minimum <= default <= maximum:
            errors.append("invalid axis bounds for "+tag)
        result.append({"tag":tag,"min":minimum,"default":default,"max":maximum})
    return result,errors

def inspect_fonts(path):
    path=Path(path)
    if path.suffix.lower() not in (".ttf",".otf",".ttc",".otc"):
        raise ValueError("Unsupported extension: "+str(path))
    if path.suffix.lower() in (".ttc",".otc"):
        collection=TTCollection(str(path),lazy=True)
        fonts=collection.fonts
    else:
        collection=None
        fonts=[TTFont(str(path),lazy=True)]
    output=[]
    try:
        for index,font in enumerate(fonts):
            try:
                variable="fvar" in font
                axes,errors=validate_axes(font["fvar"].axes) if variable else ([],[])
                report={"face":index,"variable":variable,"axes":axes,"errors":errors}
                if "STAT" not in font and variable:
                    report["warnings"]=["STAT table absent; style naming may vary by app"]
                output.append(report)
            finally:
                font.close()
    finally:
        if collection is not None: collection.close()
    return {"file":str(path),"faces":output}

def inspect_fallback(path):
    # Android 15+ can use supportedAxes on font fallback families.
    root=ET.parse(path).getroot()
    families=[]
    for family in root.findall(".//family"):
        value=family.get("supportedAxes","")
        axes=[s.strip() for s in value.split(",") if s.strip()]
        invalid=[s for s in axes if s not in ("wght","ital")]
        families.append({"name":family.get("name"),"supportedAxes":axes,
                         "errors":["unsupported fallback axis: "+x for x in invalid]})
    return {"file":str(path),"families":families}

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("fonts",nargs="*",help="TTF/OTF/TTC/OTC files to audit")
    p.add_argument("--font-fallback-xml")
    p.add_argument("--output",help="Write JSON report; otherwise print JSON")
    args=p.parse_args()
    result={"format":1,"fonts":[],"fallback":None}
    try:
        for file in args.fonts: result["fonts"].append(inspect_fonts(file))
        if args.font_fallback_xml:
            result["fallback"]=inspect_fallback(args.font_fallback_xml)
    except (ValueError, OSError, ET.ParseError) as exc:
        p.exit(2,"font audit failed: "+str(exc)+"\n")
    report=json.dumps(result,indent=2,ensure_ascii=False)
    if args.output: Path(args.output).write_text(report+"\n",encoding="utf-8")
    else: print(report)
    has_errors=any(face["errors"] for f in result["fonts"] for face in f["faces"])
    has_errors=has_errors or any(f["errors"] for f in (result["fallback"] or {}).get("families",[]))
    return 1 if has_errors else 0

if __name__=="__main__":
    raise SystemExit(main())
