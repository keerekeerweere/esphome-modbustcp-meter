# Repository Guidelines

## Project Structure & Module Organization
- `components/modbustcpmeter/` contains the custom ESPHome component sources.
- `components/modbustcpmeter/modbustcpmeter.h` is intended for the public interface and configuration types.
- `components/modbustcpmeter/modbustcpmeter.cpp` is intended for implementation details.

## Build, Test, and Development Commands
- No build, test, or lint commands are defined in this repository. The component is expected to be built and tested through a consuming ESPHome project.
- Example usage in an ESPHome config:
  - Place this repo as a custom component and reference `components/modbustcpmeter` in your ESPHome `yaml`.

## Coding Style & Naming Conventions
- Use standard C++ conventions aligned with ESPHome patterns.
- Indentation: 2 spaces, no tabs.
- File and type names should remain lowercase with underscores only when needed; keep consistency with existing filenames (example: `modbustcpmeter.cpp`).
- Keep headers minimal and focused on public API; implementation details in `.cpp`.

## Testing Guidelines
- No tests or test framework are present in this repository.
- Validate changes by integrating into an ESPHome project and running the normal ESPHome build pipeline for your target device.

## Commit & Pull Request Guidelines
- Git history is not available in this workspace, so no commit message conventions can be inferred.
- For pull requests, include:
  - A short summary of behavior changes.
  - Configuration examples or snippets used for validation.
  - Any device or Modbus register assumptions.

## Security & Configuration Tips
- Avoid hardcoding credentials or device IPs in source files.
- If adding configuration options, document defaults and any safety limits (example: timeouts, polling intervals).

## Agent-Specific Instructions
- Keep `AGENTS.md` concise and update it when adding new build steps, tests, or directories.
- If you add external tooling (lint, format, CI), document the exact commands and paths here.
