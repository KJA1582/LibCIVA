# LibCIVA Agent Guidelines

This file provides guidelines for AI agents working on the libciva codebase.

## Project Overview

LibCIVA simulates a Delco Electronics Carousel IV-A inertial navigation system (INS) with flight program CIV-A-22. It is designed for use with Microsoft Flight Simulator (FSX, MSFS, MSFS2024).

## Build Commands

### Library (CMake)
```powershell
mkdir .\out
cd .\out
cmake ..
msbuild .\libciva.sln
```

Options:
- `-DSHARED=ON` - Build shared library (default OFF)

### civaWin Example (FSX)
```powershell
cd Examples/civaWin
mkdir build && cd build
cmake ..
msbuild civaWin.sln
```

### Key Build Files
- `libciva/msfs.vcxproj` - MSFS version
- `libciva/msfs2024.vcxproj` - MSFS2024 version

### Testing
No automated test framework exists. Manual testing in the simulator is required.

---

## Code Style Guidelines

### General
- **C++ Standard**: C++14 (`set_property(TARGET libciva PROPERTY CXX_STANDARD 14)`)
- **No comments** unless explicitly requested by user
- **2-space indentation**
- **No trailing whitespace**

### Namespaces
```cpp
namespace libciva {
  // all library code
} // namespace libciva
```

### Header Guards
Traditional include guards:
```cpp
#ifndef INS_H
#define INS_H
// ...
#endif
```

### MSFS Compatibility
Use MSFS compatibility macros for IntelliSense compatibility:
```cpp
#ifndef __INTELLISENSE__
#ifndef MODULE_EXPORT
#define MODULE_EXPORT __attribute__((visibility("default")))
#define MODULE_WASM_MODNAME(mod) __attribute__((import_module(mod)))
#endif
#else
#define MODULE_EXPORT
#define MODULE_WASM_MODNAME(mod)
#define __attribute__(x)
#define __restrict__
#endif
```

### Includes
- Use **double quotes** for internal includes: `#include "ins/ins.h"`
- Use **angle brackets** for external/system includes: `#include <algorithm>`
- Sort includes in the following order (each group separated by blank line):
  1. STL or other standard lib (`<algorithm>`, `<cmath>`, `<memory>`, etc.)
  2. MSFS related (`<SimConnect.h>`, MSFS SDK headers)
  3. User lib (external libraries like libciva main import)
  4. User (within libciva or the examples)
- Sort alphabetically within each group

### Naming Conventions

| Type             | Convention                 | Example                           |
| ---------------- | -------------------------- | --------------------------------- |
| Classes          | PascalCase                 | `class INS`                       |
| Structs          | PascalCase                 | `struct POSITION`                 |
| Enums            | PascalCase                 | `enum class INS_STATE`            |
| Enum values      | PascalCase                 | `INS_STATE::NAV`                  |
| Constants        | PascalCase                 | `constexpr double MAX_BANK_ANGLE` |
| Member variables | camelCase                  | `double crossTrackError`          |
| Functions        | camelCase                  | `void updateMix() noexcept`       |
| Private helpers  | camelCase                  | `void calculateTrack() noexcept`  |
| Files            | lowercase with underscores | `insUpdates.cpp`, `varManager.h`  |
| Directories      | lowercase with underscores | `include/ins/`, `src/logger/`     |

### Enums
Use `enum class` for strong typing:
```cpp
enum class INS_STATE : uint8_t {
  OFF,
  STBY,
  ALIGN,
  NAV,
  ATT,
  FAIL,
};
```

### Structs
Use structs for POD types and data containers:
```cpp
struct POSITION {
  double latitude;
  double longitude;
  
  inline bool isValid() const noexcept { /* ... */ }
};
```

### Bitfield Unions
For hardware-like bit fields (LVars in MSFS):
```cpp
typedef union {
  double value;
  struct {
    bool MSU_BAT : 1;
    bool READY_NAV : 1;
    // ...
  } indicator;
} INDICATORS;
```

### Function Declarations
Mark functions `noexcept` where they cannot throw:
```cpp
void update(const double dTime) const noexcept;
void exportVars() const noexcept;
```

### Class Organization
Use `#pragma region` for grouping related members:
```cpp
class INS {
#pragma region Lifecycle
  INS(...) noexcept;
  ~INS() noexcept;
#pragma endregion

#pragma region Update functions
  void updatePreMix(const double dTime) noexcept;
#pragma endregion

#pragma region Events
  void handleNumeric(const uint8_t value) noexcept;
#pragma endregion
};
```

### Class Member Ordering
- Order: `private`, `protected`, `public`
- First **member variables**, then **member functions**
- Within each group: **static** members first, then non-static
- Debug-only members (e.g., logFile pointers) should be wrapped with `#ifndef NDEBUG`

### Lambda Expressions
```cpp
ins->handleEvent([event, isUnit1, isUnit2, isUnit3](auto unit1, auto unit2, auto unit3) {
  switch (event) {
    // ...
  }
});
```

### Constants
Define mathematical constants:
```cpp
constexpr double PI = 3.14159265358979323846;
constexpr double DEG2RAD = PI / 180.0;
constexpr double RAD2DEG = 180.0 / PI;
constexpr double EPSILON = 2.22044604925031308085e-16;
```

### Angle Normalization
Helper function for wrapping angles:
```cpp
static inline double absDeltaAngle(const double x, const double y) noexcept {
  constexpr auto c = 180.0;
  return c - fabs(fmod(fabs(x - y), 2 * c) - c);
}
```

### Logging
Use the Logger singleton for output:
```cpp
libciva::Logger::GetInstance() << "Message" << value << "\n";
```

---

## Important Notes

### Heading vs Track
- CIVA uses **TRUE** headings and tracks only (geodetic)
- Do NOT use magnetic headings - use `PLANE_HEADING_DEGREES_TRUE`
- `desiredTrack` is a geodetic track bearing

### Units
- Distances: nautical miles (nmi)
- Angles: degrees (true)
- Speed: knots
- Time: seconds
- Altitude: feet

### MSFS SimConnect
- `AILERON_SET` takes values -16383 to 16384
- `ROTATION VELOCITY BODY Y` is roll rate in radians/second (or deg/s depending on unit definition)
- Use `SIMCONNECT_OBJECT_ID_USER_AIRCRAFT` for the player aircraft

### Thread Safety
The gauge runs in a single-threaded context. No thread synchronization needed.

### Example Event Handling Differences

#### civa24 (MSFS Gauge)
Uses gauge events via LVar and SimConnect data callbacks:
- Events are triggered by setting `L:LIBCIVA_EVENT`
- `handleEvent()` dispatches to INS units via event IDs
- Uses `EVENT_AP_TOGGLE = 31` for autopilot toggle
- Single-threaded, runs in MSFS gauge callback context

#### civaWin (Standalone Console)
Uses direct keyboard input and thread-based architecture:
- Keyboard input via `ReadConsoleInput()` with `VK_*` virtual key codes
- Dedicated update thread with mutex-protected INS updates
- Autopilot toggle uses `'G'` key press
- Thread-safe via `std::atomic` for shared data (roll rate)

When adding features to both examples, implement the appropriate interface for each:
- **civa24**: Add event constant, handle in `handleEvent()` lambda
- **civaWin**: Add key handler in main loop switch statement

---

## File Locations

| Component                    | Path                                         |
| ---------------------------- | -------------------------------------------- |
| Main INS class               | `libciva/include/ins/ins.h`                  |
| INS implementation           | `libciva/src/ins/ins.cpp`                    |
| Type definitions             | `libciva/include/types/types.h`              |
| VarManager                   | `libciva/include/varManager/varManager.h`    |
| Logger                       | `libciva/include/logger/logger.h`            |
| Config                       | `libciva/include/state/state.h`              |
| civaWin example              | `Examples/civaWin/src/civaWin.cpp`           |
| Lateral autopilot (civaWin)  | `Examples/civaWin/src/lateralAutopilot.h`    |
| Lateral autopilot impl       | `Examples/civaWin/src/lateralAutopilot.cpp`  |
| Vertical autopilot (civaWin) | `Examples/civaWin/src/verticalAutopilot.h`   |
| Vertical autopilot impl      | `Examples/civaWin/src/verticalAutopilot.cpp` |
