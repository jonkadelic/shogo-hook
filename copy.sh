#!/bin/bash
# set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TARGET_DIR="$HOME/win7"
mkdir -p "$TARGET_DIR"

for artifact in \
  "$ROOT_DIR/build/hook_dll/shogo_hook.dll" \
  "$ROOT_DIR/build/launcher/shogo_hook_launcher.exe"
do
  if [[ -f "$artifact" ]]; then
    cp -f "$artifact" "$TARGET_DIR/"
  fi
done

cp -f "${ROOT_DIR}/build/sdlgpu_ren/sdlgpu.ren" "${TARGET_DIR}/Shogo"
cp -f "${ROOT_DIR}/build/glx_ren/glx.ren" "${TARGET_DIR}/Shogo"