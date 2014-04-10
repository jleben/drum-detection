cmake_minimum_required(VERSION 2.8)

project(uvic_mir_drum_detection)

# Find Marsyas

find_library(MARSYAS_LIB "marsyas")
find_path(MARSYAS_INCLUDE_DIR "marsyas/realvec.h")
find_program(MARSYAS_SCRIPT_CPP marsyas-script-cpp)
if(MARSYAS_LIB AND MARSYAS_INCLUDE_DIR AND MARSYAS_SCRIPT_CPP)
  set(MARSYAS_FOUND TRUE)
else()
  foreach(component MARSYAS_LIB MARSYAS_INCLUDE_DIR MARSYAS_SCRIPT_CPP)
    if(NOT ${component})
      message(STATUS "${component} not found.")
    endif()
  endforeach()
endif()

# Find Vamp SDK

find_library(VAMP_SDK_LIB "vamp-sdk")
find_path(VAMP_SDK_INCLUDE_DIR "vamp-sdk/Plugin.h")

if(VAMP_SDK_LIB AND VAMP_SDK_INCLUDE_DIR)
  set(VAMP_SDK_FOUND TRUE)
endif()

## Options

if(WIN32)
  set(base $ENV{LOCALAPPDATA})
  if(NOT base)
    set(base $ENV{HOMEPATH})
  endif()
  set(default_script_dir "${base}/marsyas/scripts")
elseif(APPLE)
  set(default_script_dir "$ENV{HOME}/Library/Application Support/marsyas/scripts")
else()
  set(default_script_dir "$ENV{HOME}/.local/share/marsyas/scripts")
endif()

set(MARSYAS_SCRIPT_DIR ${default_script_dir} CACHE PATH "Location of shared Marsyas scripts.")

##

add_subdirectory(detector)
add_subdirectory(vamp_plugins)
add_subdirectory(performance)
