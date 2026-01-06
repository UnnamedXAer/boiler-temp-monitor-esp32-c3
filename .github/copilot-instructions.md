---
description: 'Embedded C++ guidance for PlatformIO projects'
applyTo: '**/*.{cpp,c,h,hpp,hh,ino,S,s}, platformio.ini, platformio_override.ini'
---

# PlatformIO Project Instructions

## Overview

- Target modern C++17 where the platform permits; prefer `constexpr`, `enum class`, and `std::array` over dynamic allocation.
- Keep firmware non-blocking and event-driven; favor timers over `delay()` to keep loops responsive.
- Assume PlatformIO CLI workflows: `pio run`, `pio test`, `pio run -t upload`, `pio device monitor`.
- Ask before introducing new libraries; prefer built-in framework features (Arduino core, ESP-IDF, Zephyr) first.

## Project Layout

- Keep application entry at `src/main.cpp`; place reusable code in `lib/<component>/` with headers in `include/`.
- Co-locate tests under `test/` using Unity or doctest; mirror source folder names when practical.
- Store configuration in `platformio.ini`; keep per-developer overrides in `platformio_override.ini` (gitignored).
- Name environments clearly (e.g., `env:esp32-devkit`); set `default_envs` to the primary target.

## General Instructions

- Keep the `loop()` lean; offload work to state machines, timers, or RTOS tasks when available.
- Wrap hardware in small classes with clear ownership; avoid globals except for truly static hardware handles.
- Use `const` and `constexpr` aggressively; prefer `static` storage for immutable tables to avoid heap use.
- Handle errors explicitly with status enums or `std::optional`; avoid silent failures and unchecked return values.
- Minimize dynamic allocation and avoid `new`/`delete` in hot paths; use fixed-size buffers.
- Gate feature-specific code with compile-time flags and document required `build_flags`.
- Keep serial logging concise; guard verbose logging with `#ifdef DEBUG` or log-level macros.

## Code Standards

- Naming: classes/types `PascalCase`, functions and variables `camelCase`, constants and macros `UPPER_SNAKE_CASE`.
- Prefer `#pragma once` for headers; order includes: corresponding header, project headers, framework headers, C/STD, then STL.
- Keep headers minimal; use forward declarations to cut compile times and include ripple.
- Use `auto` when the type is obvious from the right-hand side; keep explicit types for hardware registers and bitfields.
- Prefer range-based for loops, `std::span`/`std::array` for buffer handling, and strongly typed `enum class` for states.
- Document timing-critical sections with brief comments; avoid redundant commentary.

## Configuration (platformio.ini)

- Group shared settings in `[platformio]` and `[common]` extra sections; reuse via `${common.*}`.
- Pin platforms, frameworks, and libraries to known-good versions; avoid floating versions.
- Set upload and monitor speeds explicitly; include `upload_port` only when necessary to avoid developer conflicts.
- Keep `build_flags` readable; prefer one flag per line and add a short note when non-obvious.

## Best Practices

- Avoid blocking calls (`delay`, long while-loops); schedule using `millis()` deltas or framework timers.
- Use RAII wrappers for peripherals (e.g., serial, SPI transactions) to ensure cleanup on error paths.
- Debounce inputs in software or via hardware RC; avoid tight polling without backoff.
- Guard ISR code: keep ISRs short, no heap, no blocking, and communicate via ring buffers or queues.
- Separate configuration from code: move pins, credentials, and tuning constants into headers or `platformio.ini` variables.

## Common Patterns

### Non-blocking Blink (Good)
```cpp
constexpr uint32_t kBlinkIntervalMs = 500;
uint32_t lastToggleMs = 0;

void loop() {
	const uint32_t now = millis();
	if (now - lastToggleMs >= kBlinkIntervalMs) {
		lastToggleMs = now;
		digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
	}
	// Handle other work here without blocking.
}
```

### Blocking Blink (Bad)
```cpp
void loop() {
	digitalWrite(LED_BUILTIN, HIGH);
	delay(500);
	digitalWrite(LED_BUILTIN, LOW);
	delay(500);
}
```

### State Machine Skeleton
```cpp
enum class AppState { Idle, Connecting, Running, Error };

AppState state = AppState::Idle;

void loop() {
	switch (state) {
		case AppState::Idle:
			// transition conditions
			break;
		case AppState::Connecting:
			// attempt connect with timeouts
			break;
		case AppState::Running:
			// normal operation
			break;
		case AppState::Error:
			// safe recovery or reset
			break;
	}
}
```

## Testing and Verification

- Add unit tests under `test/`; keep fixtures minimal and deterministic.
- Prefer host-based tests for pure logic; hardware-in-the-loop tests should be clearly marked and isolated.
- Use fakes or small adapters to mock hardware dependencies in tests.

## Validation Commands

- Build: `pio run`
- Upload: `pio run -t upload`
- Unit tests: `pio test`
- Monitor: `pio device monitor -b <baud>`
- Clean: `pio run -t clean`
- `pio` is not available system wide, use full path for the commands like: `& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -d "d:\platformio\boiler-temp-monitor-esp32-c3"`

## Security and Reliability

- Never commit secrets; Use a separate `secrets.h` file that is excluded from version control via `.gitignore`. (maintain a copy of the `secrets.h` file as `secrets_copy.h` with placeholder values for reference.)
- Validate external inputs (serial, network, sensor data); clamp to safe ranges.
- Fail safe: on unrecoverable errors, put outputs in a safe state before reset.
- Watchdog: enable and feed watchdog timers in long-running code paths where available.

## Project Requirements

- Consult `docs/requirements.md` for project requirements and goals.