# CM Emulation project.
## Prerequisites

- [CMake](https://cmake.org/), version >= 3.10
- [pkg-config](https://github.com/freedesktop/pkg-config), latest version.
- [libffi](https://github.com/libffi/libffi), latest version.
- [OpenCL headers](https://github.com/KhronosGroup/OpenCL-Headers), latest version.
- [Level Zero development package](https://github.com/intel/compute-runtime/releases), latest version.
- [LibVA development package](https://github.com/intel/libva) with API version >= 1.6.0 (see va_version.h for **VA_VERSION**)

## Build instructions
### Linux
1. Get the source code and put to \<PATH TO SOURCE>
2. **mkdir** \<BUILD DIRECTORY> && **cd** \<BUILD DIRECTORY>
3. **cmake** [OPTIONS, see below] \<PATH TO SOURCE>

> By default **system-installed** headers and libraries shall be used. 
> To override some locations the following **cmake options** can be used
>
> - ### LibVA installation prefix:
>
> cmake -D**LIBVA_INSTALL_PATH**=\<e.g. /usr>
>
> - ### Level Zero headers and lib path:
>
> cmake -D**LevelZero_INCLUDE_DIR**=\<PATH CONTAINING level_zero subdir with headers>
>
> cmake -D**LevelZero_LIBRARY**=\<PATH TO>libze_loader.so
>
> - ### Custom installation path can be set with 
>
> cmake -D**CMAKE_INSTALL_PREFIX**=\<YOUR PATH>

4. **make**

#### Make packages

> To create DEB, RPM packages and TAR.XZ archive in \<BUILD DIRECTORY> run

5. **make** package

## Installation
### Linux

1. **make** install

## License

This project is licensed under the MIT License. You may obtain a copy of the License at:

https://opensource.org/licenses/MIT


