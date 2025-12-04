# Font Scaling Feature

## Overview
PrismaOS now includes dynamic font size adjustment commands, similar to Arch Linux terminal font scaling.

## Commands

### `fontsize+`
Increases the font size by adjusting the VGA text mode scan lines.
- **Small → Normal**: Changes from 8x16 to 8x14 font
- **Normal → Large**: Changes from 8x14 to 8x8 font (fewer scan lines = bigger characters)

### `fontsize-`
Decreases the font size by adjusting the VGA text mode scan lines.
- **Large → Normal**: Changes from 8x8 to 8x14 font
- **Normal → Small**: Changes from 8x14 to 8x16 font (more scan lines = smaller characters)

## Usage

Type the command in the PrismaOS shell:

```
PrimusOS> fontsize+
Font size: Large

PrimusOS> fontsize-
Font size: Normal
```

## Technical Details

The font scaling feature works by programming the VGA CRTC (Cathode Ray Tube Controller) registers:

- **Register 0x3D4/0x3D5**: CRT Controller Address/Data registers
- **Index 0x09**: Maximum Scan Line Register

The implementation modifies the scan line height:
- **Small font**: 16 scan lines per character (8x16)
- **Normal font**: 14 scan lines per character (8x14) 
- **Large font**: 8 scan lines per character (8x8)

## Files Modified

- `include/tty.h` - Added font scale type definition and function declarations
- `src/tty.c` - Implemented VGA font height adjustment functions
- `src/kernel.c` - Added command handlers for `fontsize+` and `fontsize-`

## Compatibility

This feature works with standard VGA text mode (mode 3) and is compatible with:
- Real hardware with VGA-compatible video cards
- QEMU and other virtual machines with VGA emulation
- Most x86 bootloaders that initialize text mode
