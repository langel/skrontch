# Skrontch

A suite of tools in a single application for creating and manipulating content and media for homebrew video game projects, focusing primarily on graphics and sound.

## Overview

Skrontch provides an integrated workspace for game development content creation, featuring a flexible GUI system built on windows, tabs, and split panes. This allows users to arrange their workspace exactly as needed for their workflow.

## Technology Stack

- **Language**: C (C99 standard)
- **Framework**: SDL2
- **Platform**: Cross-platform (Windows, Linux, macOS)

## Project Goals

- Provide multiple tools in a single unified application
- Focus on graphics and sound manipulation for homebrew game projects
- Flexible, customizable workspace layout
- Fast, responsive interface
- Extensible architecture for adding new tools

## Building

### Windows (batch)

1. Download the SDL2 development package and extract it.
2. Set `SDL2_DIR` to the extracted folder:
   - Example: `set SDL2_DIR=C:\libs\SDL2`
3. Build:
   - `build.bat`
4. Run:
   - `run.bat`

If `SDL2.dll` is found in your SDL2 `lib` folder, `build.bat` will copy it into `build/`.

### macOS/Linux (bash)

1. Install SDL2 + SDL2_image dev tools (e.g., `libsdl2-dev` and `libsdl2-image-dev`).
2. Build:
   - `./build.sh`
3. Run:
   - `./run.sh`

## Architecture

See [ARCHITECTURE.md](ARCHITECTURE.md) for detailed system design.

## Development

- All code uses **snake_case** naming convention
- See `.cursorrules` for detailed coding standards and patterns
- Build incrementally: core systems first, then GUI components, then tools

## License

(To be determined)
