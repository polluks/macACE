# Install script for directory: /Users/phillipb/Documents/macACE/deps/ace

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/Users/phillipb/.vscode/extensions/bartmanabyss.amiga-debug-1.7.9/bin/darwin/opt/usr")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Users/phillipb/.vscode/extensions/bartmanabyss.amiga-debug-1.7.9/bin/darwin/opt/bin/m68k-amiga-elf-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/phillipb/Documents/macACE/build/_deps/bartman_gcc_support-build/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/objects-Release/ace" TYPE FILE FILES
    "src/ace/managers/advancedsprite.c.obj"
    "src/ace/managers/blit.c.obj"
    "src/ace/managers/bob.c.obj"
    "src/ace/managers/copper.c.obj"
    "src/ace/managers/game.c.obj"
    "src/ace/managers/joy.c.obj"
    "src/ace/managers/key.c.obj"
    "src/ace/managers/log.c.obj"
    "src/ace/managers/memory.c.obj"
    "src/ace/managers/mouse.c.obj"
    "src/ace/managers/ptplayer.c.obj"
    "src/ace/managers/rand.c.obj"
    "src/ace/managers/sprite.c.obj"
    "src/ace/managers/state.c.obj"
    "src/ace/managers/system.c.obj"
    "src/ace/managers/timer.c.obj"
    "src/ace/managers/viewport/camera.c.obj"
    "src/ace/managers/viewport/scrollbuffer.c.obj"
    "src/ace/managers/viewport/simplebuffer.c.obj"
    "src/ace/managers/viewport/tilebuffer.c.obj"
    "src/ace/utils/bitmap.c.obj"
    "src/ace/utils/bmframe.c.obj"
    "src/ace/utils/chunky.c.obj"
    "src/ace/utils/custom.c.obj"
    "src/ace/utils/dir.c.obj"
    "src/ace/utils/disk_file.c.obj"
    "src/ace/utils/extview.c.obj"
    "src/ace/utils/file.c.obj"
    "src/ace/utils/font.c.obj"
    "src/ace/utils/pak_file.c.obj"
    "src/ace/utils/palette.c.obj"
    "src/ace/utils/sprite.c.obj"
    "src/ace/utils/string.c.obj"
    "src/ace/utils/tag.c.obj"
    "src/fixmath/fix16.c.obj"
    "src/fixmath/fix16_exp.c.obj"
    "src/fixmath/fix16_sqrt.c.obj"
    "src/fixmath/fix16_str.c.obj"
    "src/fixmath/fix16_trig.c.obj"
    "src/fixmath/fract32.c.obj"
    "src/fixmath/uint32.c.obj"
    "src/mini_std/ctype.c.obj"
    "src/mini_std/errno.c.obj"
    "src/mini_std/intrin.c.obj"
    "src/mini_std/printf.c.obj"
    "src/mini_std/stdio_file.c.obj"
    "src/mini_std/stdio_putchar.c.obj"
    "src/mini_std/stdlib.c.obj"
    "src/mini_std/string.c.obj"
    "src/mini_std/strtoul.c.obj"
    FILES_FROM_DIR "/Users/phillipb/Documents/macACE/build/ace/CMakeFiles/ace.dir/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/Users/phillipb/Documents/macACE/build/ace/CMakeFiles/ace.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ace" TYPE FILE FILES
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/macros.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/types.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ace/generic" TYPE FILE FILES
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/generic/main.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/generic/screen.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ace/utils" TYPE FILE FILES
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/utils/bitmap.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/utils/bmframe.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/utils/chunky.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/utils/custom.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/utils/dir.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/utils/disk_file.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/utils/disk_file_private.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/utils/endian.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/utils/extview.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/utils/file.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/utils/font.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/utils/mini_std.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/utils/pak_file.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/utils/palette.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/utils/sprite.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/utils/string.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/utils/tag.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ace/managers" TYPE FILE FILES
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/advancedsprite.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/blit.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/bob.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/copper.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/game.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/joy.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/key.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/log.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/memory.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/mouse.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/ptplayer.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/rand.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/sprite.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/state.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/system.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/timer.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ace/managers/viewport" TYPE FILE FILES
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/viewport/camera.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/viewport/scrollbuffer.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/viewport/simplebuffer.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/ace/managers/viewport/tilebuffer.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/fixmath" TYPE FILE FILES
    "/Users/phillipb/Documents/macACE/deps/ace/include/fixmath/fix16.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/fixmath/fix16_trig_sin_lut.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/fixmath/fixmath.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/fixmath/fract32.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/fixmath/int64.h"
    "/Users/phillipb/Documents/macACE/deps/ace/include/fixmath/uint32.h"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/phillipb/Documents/macACE/build/ace/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
