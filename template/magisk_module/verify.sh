TMPDIR_FOR_VERIFY="$TMPDIR/.vunzip"
mkdir "$TMPDIR_FOR_VERIFY"

abort_verify() {
  ui_print "*********************************************************"
  ui_print "! $1"
  ui_print "! This zip may be corrupted, please try downloading again"
  abort    "*********************************************************"
}

# extract <zip> <file> <target dir> <junk paths>
extract() {
  zip=$1
  file=$2
  dir=$3
  junk_paths=$4
  [ -z "$junk_paths" ] && junk_paths=false
  opts="-o"
  [ $junk_paths = true ] && opts="-oj"

  file_path=""
  hash_path=""
  if [ $junk_paths = true ]; then
    file_path="$dir/$(basename "$file")"
    hash_path="$TMPDIR_FOR_VERIFY/$(basename "$file").sha256sum"
  else
    file_path="$dir/$file"
    hash_path="$TMPDIR_FOR_VERIFY/$file.sha256sum"
  fi

  unzip $opts "$zip" "$file" -d "$dir" >&2
  [ -f "$file_path" ] || abort_verify "$file not exists"

  unzip $opts "$zip" "$file.sha256sum" -d "$TMPDIR_FOR_VERIFY" >&2
  [ -f "$hash_path" ] || abort_verify "$file.sha256sum not exists"

  # Compare SHA-256 digests directly: Android Toybox, BusyBox and GNU
  # sha256sum differ in the support for '-s' / '--status'.
  expected_hash="$(cat "$hash_path")"
  actual_line="$(sha256sum "$file_path")" || abort_verify "Cannot hash $file"
  actual_hash="${actual_line%% *}"
  if [ -z "$expected_hash" ] || [ "$actual_hash" != "$expected_hash" ]; then
    abort_verify "SHA-256 mismatch for $file"
  fi
  ui_print "- Verified $file" >&1
}
