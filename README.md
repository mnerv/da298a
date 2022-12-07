# Engineering Project with Software Engineering

The project consist of hardware, firmware and software stacks. **sky** is a framework library for both **shelter** and **sunlight**. **shelter** being the sandbox simulation program without the hardware. **sunlight** is firmware for the hardware(**tempos**).

To run the whole software stack follow the requirement list.

## Requirements

  - [cmake](https://cmake.org)
  - [vcpkg](http://vcpkg.io)
  - [PlatformIO](https://platformio.org/)
  - [KiCAD](https://www.kicad.org/)

This list is for the whole project and is not required if only subset of the project is in used. Check the different guides on what is actually required.

## Development Guide (nurture)

To run the simulation program it is required that [cmake](https://cmake.org) and [vcpkg](http://vcpkg.io) is installed. `cmake` is used for setting up build script for the current working environment and `vcpkg` is used for package management to automatically download required libraries to build.

Follow [Get started with vcpkg](https://vcpkg.io/en/getting-started.html) guide on how to download and setup `vcpkg` for all systems.

### Ninja (macOS and Linux)

This requires that `VCPKG_ROOT` environment variable is defined.

```
cmake -S . -Bbuild -GNinja -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
```

`${VCPKG_ROOT}`: This points to where the `vcpkg` is installed.

### macOS (Xcode)

This requires that `VCPKG_ROOT` environment variable is defined.

```
cmake -S . -Bbuild -GXcode -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
```

`${VCPKG_ROOT}`: This points to where the `vcpkg` is installed.

### Windows (Visual Studio)

This requires [Visual Studio](https://visualstudio.microsoft.com/). Use command below to generate Visual Studio solution files. `${VCPKG_ROOT}` needs to be replaced with path to where vcpkg is installed.

```
cmake -S . -Bbuild -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
```

## Development Guide (sunlight)

To build and upload the firmware to the hardware [PlatformIO](https://platformio.org/) is required. Install the [PlatformIO IDE extension](https://platformio.org/platformio-ide) for Visual Studio Code and open the `sunlight` directory and the build environment should be automatically configured.
