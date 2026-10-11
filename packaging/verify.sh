verify_extract() {
  _name="$1"
  _dest="$2"
  _wanted="$(unzip -p "$ZIPFILE" "$_name.sha256" 2>/dev/null)" || abort "! Checksum missing"
  [ -n "$_wanted" ] || abort "! Empty checksum"
  unzip -p "$ZIPFILE" "$_name" > "$_dest" || abort "! Entry missing"
  _actual="$(sha256sum "$_dest" | cut -d ' ' -f 1)" || abort "! sha256sum failed"
  [ "$_wanted" = "$_actual" ] || abort "! SHA256 mismatch"
  ui_print "- Verified $_name"
}
