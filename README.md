# CoreOS

**CoreOS** is a lightweight, 32-bit, ring-0 operating system written from scratch with a focus on education, experimentation, and fun. It features a custom shell, dynamic memory management, cryptography utilities, mathematical functions, and interactive commands, providing a foundation for building OS-level experiments and simulators.

---

## Table of Contents

- [Features](#features)  
- [Installation](#installation)  
- [Usage](#usage)  
- [Commands](#commands)  
- [Contributors](#contributors)  
- [License](#license)  
- [Future Plans](#future-plans)

---

## Features

- **Custom Shell**: A fully interactive kernel shell with support for command history, colored output, and dynamic input handling.  
- **Dynamic Memory Management**: `kmalloc` and `kfree` for heap allocation inside the kernel.  
- **Math Utilities**: Support for common mathematical functions (`sqrt`, `pow`, `sin`, `cos`, `log`, etc.) and arithmetic computations.  
- **Cryptography Utilities**: SHA-224 and SHA-256 hashing directly in the kernel.  
- **Interactive Fun Commands**: Easter eggs, jokes, ASCII animations, and interactive chatbot-like responses.  
- **Version Management**: Automatic patch version incrementation and `version.h` generation.  
- **Terminal Features**: Customizable colors, prompt, and text output using `printk`.  
- **Keyboard Handling**: Supports shift, capslock, and special keys with real-time feedback.  

---

## Installation

### Prerequisites

- `gcc`, `nasm`, `as`, `ld`  
- `grub-mkrescue` for ISO creation  
- Linux or WSL environment recommended

### Build Instructions

1. Clone the repository:

```bash
git clone https://github.com/Teams-Workspace/CoreOS.git
cd CoreOS
```

### Contributors

- **Saad Ali – Ghazanfar Pasha - Roshaan Idrees**


### GitHub Profiles:

`0X1Saad`, `itsmesaadali`, `Ghazanfar Pasha`, `Muhammad Roshaan Idrees`

## Running in QEMU (scaled text mode)

To run the built ISO in a window that scales the VGA text display (keeps text crisp and left-aligned), use the bundled script:

```bash
./run_qemu.sh
```

Notes:
- The script uses SDL display with window controls (minimize, maximize, close buttons).
- Sets `SDL_RENDER_SCALE_QUALITY=nearest` so when you resize the window, VGA text scales with nearest-neighbor (no blur).
- You can toggle fullscreen in QEMU (SDL) with `Ctrl+Alt+F` (host/window-manager dependent).
- The UI includes color-coded text: cyan logo, grey info, yellow hints, green prompt.
- If you prefer true fullscreen at start, run `qemu-system-x86_64 ... -display sdl -full-screen` (some environments require X11/SDL support).
