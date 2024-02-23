# Use this file for windows compilation with the 
#`-DCMAKE_TOOLCHAIN_FILE=windows_toolchain.cmake` flag

# https://cmake.org/cmake/help/book/mastering-cmake/chapter/Cross%20Compiling%20With%20CMake.html

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_CXX_COMPILER x86_64-w32-mingw32-g++) # TODO: Set this right

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
