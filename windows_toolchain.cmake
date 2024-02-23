# Use this file for windows compilation with the 
#`-DCMAKE_TOOLCHAIN_FILE=windows_toolchain.cmake` flag

# https://cmake.org/cmake/help/book/mastering-cmake/chapter/Cross%20Compiling%20With%20CMake.html

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
