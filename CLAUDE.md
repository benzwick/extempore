# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

Extempore is a cyberphysical programming environment that combines a Scheme interpreter with XTLang (eXTempore Language), a statically-typed language that compiles to LLVM IR at runtime. It's designed for live coding, especially in music and audio-visual performance contexts.

**Note**: Extempore is not yet compatible with Apple Silicon.

## Build System

Extempore uses CMake and builds LLVM 3.8.0 as part of its compilation process.

### Building from Source

**On Linux/macOS**:
```bash
mkdir build && cd build
cmake -DASSETS=ON .. && make && sudo make install
```

**On Windows** (VS2019 example):
```bash
mkdir build && cd build
cmake -G "Visual Studio 16 2019" -A x64 -DASSETS=ON .. && cmake --build . --target INSTALL --config Release
```

**Important CMake Options**:
- `-DASSETS=ON`: Downloads ~300MB of assets (audio files, 3D models) required for many examples
- `-DEXTERNAL_SHLIBS_AUDIO=ON` (default): Build audio-related external libraries
- `-DEXTERNAL_SHLIBS_GRAPHICS=ON` (default): Build graphics-related external libraries
- `-DEXT_DYLIB=ON`: Build Extempore as a dynamic library instead of executable
- `-DBUILD_TESTS=ON` (default): Build test targets
- `-DJACK=ON`: Use Jack audio backend (Linux only, experimental)

### Running Tests

```bash
# From build directory
ctest

# Run specific test labels
ctest -L libs-core        # Core library tests
ctest -L libs-external    # External library tests
ctest -L examples-audio   # Audio examples
ctest -L examples-graphics # Graphics examples
```

### AOT Compilation

Extempore supports ahead-of-time (AOT) compilation of XTLang libraries for faster startup:

```bash
# AOT compile core libraries (done automatically during build)
make aot_core

# Clean AOT-compiled libraries
make clean_aot
```

### Platform-Specific Dependencies

**Linux (Ubuntu/Debian)**:
```bash
sudo apt-get install libasound2-dev xorg-dev libglu1-mesa-dev
```

**macOS**: Requires Command Line Tools. On macOS 10.14+, you may need to install system headers:
```bash
sudo installer -pkg /Library/Developer/CommandLineTools/Packages/macOS_SDK_headers_for_macOS_10.14.pkg -target /
```

**Windows**: Requires Visual Studio 2017 or 2019. For ASIO support, download the Steinberg ASIO SDK separately.

## Architecture

### Core Components

**C++ Runtime** (`src/`):
- `Extempore.cpp` - Main entry point and command-line argument parsing
- `SchemeProcess.cpp` - Manages Scheme interpreter instances (primary and utility processes)
- `SchemeREPL.cpp` - Network REPL server for live interaction
- `EXTLLVM.cpp` - LLVM JIT compilation infrastructure
- `AudioDevice.cpp` - Audio I/O via PortAudio
- `TaskScheduler.cpp` - Real-time scheduling for audio callbacks
- `OSC.cpp` - OSC (Open Sound Control) message handling

**Scheme Layer** (`runtime/`):
- `scheme.xtm` - Core Scheme implementation
- `init.xtm` - Bootstrap code, loads at startup
- `llvmti.xtm` - LLVM type information and bindings
- `llvmir.xtm` - LLVM IR generation from XTLang

**Library Structure** (`libs/`):
- `base/` - Foundation libraries (base.xtm, adt.xtm, pattern.xtm)
- `core/` - Pure XTLang libraries with no external C dependencies:
  - `math.xtm` - Mathematical functions
  - `audio_dsp.xtm` - DSP primitives and audio processing
  - `instruments.xtm` - Synthesizer instruments
  - `audiobuffer.xtm` - Audio buffer management
  - `rational.xtm` - Rational number arithmetic
- `external/` - XTLang bindings to external C libraries:
  - Audio: `portmidi.xtm`, `sndfile.xtm`, `fft.xtm`
  - Graphics: `glfw3.xtm`, `nanovg.xtm`, `assimp.xtm`, `gl/`
  - System: `opengl.xtm`, `portaudio.xtm`, `sqlite.xtm`
- `contrib/` - Community-contributed libraries

### Two-Process Model

Extempore runs two Scheme processes by default:
1. **Primary process** (port 7099) - User's main interaction point, runs on thread 0
2. **Utility process** (port 7098) - Background process for system tasks

Both processes can be connected to via TCP/IP for live coding interaction.

### LLVM Integration

Extempore uses LLVM 3.8.0 for JIT compilation:
- XTLang code compiles to LLVM IR at runtime
- Custom calling conventions for real-time audio callbacks
- Closures are heap-allocated with zone-based memory management
- AOT compilation generates shared objects (`.so`/`.dll`) in `libs/aot-cache/`

### Memory Management

XTLang uses zone-based memory allocation:
- Zones are memory regions that can be deallocated atomically
- `llvm_zone_t*` is passed to closures for allocation context
- No garbage collection; manual zone creation/destruction
- Zones can be marked for delayed destruction (after audio callbacks complete)

## Development Workflow

### Running Extempore

```bash
# Start with default settings (loads base.xtm)
./extempore

# Start on different port
./extempore --port 7100

# Start without loading base library
./extempore --nobase

# Start without audio
./extempore --noaudio

# Evaluate expression on startup
./extempore --eval "(println 'hello)"

# Run a file on startup
./extempore --run path/to/file.xtm

# Set audio device
./extempore --print-devices    # List available devices
./extempore --device 2          # Use device index 2
./extempore --device-name "USB" # Match device by name (regex)
```

### Connecting to Extempore

Once Extempore is running, connect via:
- VSCode Extempore extension (recommended)
- Emacs with extempore-mode
- Telnet: `telnet localhost 7099`
- Custom editor with TCP socket connection

### File Extensions

- `.xtm` - Extempore files (can contain both Scheme and XTLang code)
- `.scm` - Pure Scheme files

### Common Command-Line Options

- `--sharedir <path>` - Override location of runtime/libs/examples directories
- `--samplerate <rate>` - Set audio sample rate
- `--frames <n>` - Set audio buffer size (default 1024)
- `--channels <n>` - Set number of output audio channels
- `--inchannels <n>` - Set number of input audio channels
- `--arch <arch>` - Target architecture for LLVM (default: host)
- `--cpu <cpu>` - Target CPU for LLVM (default: host)
- `--attr <attr>` - Additional LLVM target attributes
- `--compile <file.xtm>` - Compile XTLang to native executable (AOT)
- `--term <type>` - Terminal type: `ansi`, `cmd`, `basic`, or `nocolor`

## Code Style Notes

### C++ Code

- Uses C++11 standard
- LLVM requires `-fno-rtti` (no RTTI)
- Platform-specific code sections for Windows/macOS/Linux
- Windows uses Objective-C++ for some source files (`.cpp` with `-x objective-c++`)

### XTLang Conventions

- Type annotations use `:` syntax: `(let ((x:i64 5))...)`
- Generic functions use angle brackets: `<T>`
- Closures capture environment: `(lambda (x:i64) (lambda () x))`
- Bind statement for type inference: `(bind-func func_name (lambda ...))`

### Scheme Conventions

- Traditional Scheme with some extensions
- Pattern matching via `pattern.xtm`
- Temporal recursion for scheduling: `(callback (+ time 44100) func ...)`

## Testing

Tests are located in `tests/`:
- `tests/core/` - Core XTLang and Scheme tests
- `tests/external/` - External library tests

Examples in `examples/` also serve as integration tests:
- `examples/core/` - Core functionality examples
- `examples/external/` - External library examples

## Important Implementation Details

### Signal Handling

- SIGINT/SIGTERM handlers for clean shutdown on Unix platforms
- Windows uses `SetConsoleCtrlHandler` for Ctrl-C handling
- Handlers defined in `Extempore.cpp`

### Audio Thread Safety

- Audio callbacks run on real-time thread
- Use lock-free data structures when communicating with audio thread
- Zone destruction can be delayed to avoid real-time thread allocation

### Windows-Specific

- Target triple requires "-elf" suffix for MCJIT
- Uses experimental `<filesystem>` (not `std::filesystem`) due to LLVM 3.8 constraints
- MSVC defines required: `PCRE_STATIC`, `_CRT_SECURE_NO_WARNINGS`

### macOS-Specific

- Uses Cocoa/AppKit frameworks for GUI integration
- `NSApplication sharedApplication` called without starting run loop
- Clang/Clang++ are default compilers
- Uses frameworks: Cocoa, CoreAudio, AudioUnit, AudioToolbox

## Documentation and Resources

- Online documentation: https://extemporelang.github.io/docs/
- Mailing list: http://groups.google.com/group/extemporelang
- Chat: #extempore on chat.toplap.org
- GitHub Actions: Automatic builds and tests on push
