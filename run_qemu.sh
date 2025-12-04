#!/usr/bin/env bash
# Helper script to run PrimusOS in QEMU with fixed font size (no text stretching).
# Window can be resized but VGA text remains at original pixel size.

set -euo pipefail

ISO="primus-os.iso"
if [ ! -f "$ISO" ]; then
  echo "Error: $ISO not found. Build the ISO first (make)."
  exit 1
fi

# Use SDL display with integer scaling to prevent font distortion.
# VGA text mode is 720x400 pixels (80x25 chars). 
# Scale 2x for better readability while keeping pixels sharp.
export SDL_RENDER_SCALE_QUALITY=nearest
export SDL_VIDEO_X11_FORCE_EGL=0
qemu-system-x86_64 \
  -cdrom "$ISO" \
  -serial stdio \
  -vga std \
  -display sdl,window-close=on,gl=off \
  -global VGA.vgamem_mb=16 \
  -global VGA.edid=off \
  "$@"

# Note: VGA text mode has FIXED 80x25 resolution that cannot be changed.
# Font may scale with window size - this is normal QEMU behavior.

# Alternative: Use SDL with window scaling disabled
# export SDL_VIDEO_ALLOW_SCREENSAVER=1
# qemu-system-x86_64 -cdrom "$ISO" -serial stdio -vga std -display sdl,window-close=on "$@"
