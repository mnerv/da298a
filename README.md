# Engineering Project with Software Engineering

## Requirements

  - [cmake](https://cmake.org)
  - [vcpkg](http://vcpkg.io)

## Development

```
cmake -S . -Bbuild -GNinja -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
```

`${VCPKG_ROOT}`: This points to where the `vcpkg` is installed.
