cmake_minimum_required(VERSION 2.8)

project(uvic_mir_drum_detection)

# Find Marsyas

find_library(MARSYAS_LIB "marsyas")
find_path(MARSYAS_INCLUDE_DIR "marsyas/realvec.h")
if(MARSYAS_LIB AND MARSYAS_INCLUDE_DIR)
  set(MARSYAS_FOUND TRUE)
endif()

# Find Vamp SDK

find_library(VAMP_SDK_LIB "vamp-sdk")
find_path(VAMP_SDK_INCLUDE_DIR "vamp-sdk/Plugin.h")

if(VAMP_SDK_LIB AND VAMP_SDK_INCLUDE_DIR)
  set(VAMP_SDK_FOUND TRUE)
endif()

##

add_subdirectory(detector)
add_subdirectory(vamp_plugins)
add_subdirectory(performance)
