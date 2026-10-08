#!/system/bin/sh
# Informational only; root provider does not need to expose the Magisk CLI.
MODDIR=${0%/*}
if command -v log >/dev/null 2>&1; then
  log -p i -t FontLoader "Module active: $MODDIR"
fi
