cmake_minimum_required(VERSION 3.24)

if(PLATFORM_NAME STREQUAL "Windows64")
  project(MinecraftConsoles LANGUAGES C CXX RC ASM_MASM)
else()
  project(MinecraftConsoles LANGUAGES C CXX)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32 AND NOT (CMAKE_SYSTEM_NAME STREQUAL "Linux"))
  message(FATAL_ERROR "Unsupported platform. Only Windows and Linux are supported.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "Use a 64-bit generator/toolchain (x64).")
endif()

set(CMAKE_CONFIGURATION_TYPES
  "Debug"
  "Release"
  CACHE STRING "" FORCE
)
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(PLATFORM_NAME STREQUAL "Linux64")
  find_package(OpenGL REQUIRED)
  find_package(GLEW REQUIRED)
  find_package(SDL2 QUIET CONFIG)
  if(NOT SDL2_FOUND)
    find_package(SDL2 REQUIRED)
  endif()
  find_package(Threads REQUIRED)

  if(TARGET SDL2::SDL2)
    set(MINECRAFT_LINUX_SDL2_TARGET SDL2::SDL2)
  elseif(TARGET SDL2::SDL2-static)
    set(MINECRAFT_LINUX_SDL2_TARGET SDL2::SDL2-static)
  else()
    message(FATAL_ERROR "SDL2 was found, but no usable SDL2::SDL2 target is available.")
  endif()

  if(TARGET OpenGL::GL)
    set(MINECRAFT_LINUX_OPENGL_TARGET OpenGL::GL)
  elseif(TARGET OpenGL::OpenGL)
    set(MINECRAFT_LINUX_OPENGL_TARGET OpenGL::OpenGL)
  else()
    message(FATAL_ERROR "OpenGL was found, but no usable OpenGL imported target is available.")
  endif()
endif()

function(configure_compiler_target target)
  # MSVC and compatible compilers (like Clang-cl)
  if (MSVC)
    target_compile_options(${target} PRIVATE
      $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C,CXX>>:/W3>
      $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C,CXX>>:/W0>
      $<$<COMPILE_LANGUAGE:C,CXX>:/MP>
      $<$<COMPILE_LANGUAGE:C,CXX>:/FS>
      $<$<COMPILE_LANGUAGE:C,CXX>:/GS>
      $<$<COMPILE_LANGUAGE:CXX>:/EHsc>
      $<$<COMPILE_LANGUAGE:CXX>:/GR>
      $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C,CXX>>:/Od>
      $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C,CXX>>:/O2 /Oi /GT /GF>
    )
  endif()

  # MSVC
  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${target} PRIVATE
      $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C,CXX>>:/GL>
    )
    target_link_options(${target} PRIVATE
      $<$<CONFIG:Release>:/LTCG:incremental>
    )
  endif()

  # Clang
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${target} PRIVATE
      $<$<AND:$<STREQUAL:${PLATFORM_NAME},Linux64>,$<COMPILE_LANGUAGE:CXX>>:-fdeclspec -fms-extensions>
      $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C,CXX>>:-O0 -Wall>
      $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C,CXX>>:-O2 -w -flto>
    )
    target_link_options(${target} PRIVATE
      $<$<CONFIG:Release>:-flto>
    )
  endif()

  # GCC (Linux)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${target} PRIVATE
      $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C,CXX>>:-O0 -Wall -Wno-unused-result>
      $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C,CXX>>:-O2 -w>
    )
  endif()
endfunction()


# ---
# Configuration
# ---
set(MINECRAFT_SHARED_DEFINES
  _LARGE_WORLDS
  _DEBUG_MENUS_ENABLED
  $<$<CONFIG:Debug>:_DEBUG>
  _CRT_NON_CONFORMING_SWPRINTFS
  _CRT_SECURE_NO_WARNINGS
)

# Add platform-specific defines
list(APPEND MINECRAFT_SHARED_DEFINES ${PLATFORM_DEFINES})

# ---
# Sources
# ---
add_subdirectory(Minecraft.World)
add_subdirectory(Minecraft.Client)
if(PLATFORM_NAME STREQUAL "Windows64")
  add_subdirectory(Minecraft.Server)
endif()

# ---
# Build versioning
# ---
set(BUILDVER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateBuildVer.cmake")
set(BUILDVER_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/Common/BuildVer.h")

add_custom_command(
  OUTPUT "${BUILDVER_OUTPUT}"
  COMMAND ${CMAKE_COMMAND}
    "-DOUTPUT_FILE=${BUILDVER_OUTPUT}"
    -P "${BUILDVER_SCRIPT}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  COMMENT "Generating BuildVer.h..."
  VERBATIM
)

add_custom_target(GenerateBuildVer DEPENDS "${BUILDVER_OUTPUT}")

add_dependencies(Minecraft.World GenerateBuildVer)
add_dependencies(Minecraft.Client GenerateBuildVer)
if(PLATFORM_NAME STREQUAL "Windows64")
  add_dependencies(Minecraft.Server GenerateBuildVer)
endif()

# ---
# Project organisation
# ---
# Set the startup project for Visual Studio
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Minecraft.Client)

# Setup folders for Visual Studio, just hides the build targets under a sub folder
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(TARGET GenerateBuildVer PROPERTY FOLDER "Build")
