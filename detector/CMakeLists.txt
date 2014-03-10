if(NOT MARSYAS_FOUND)
  message(STATUS "Not building detection app. (Marsyas not found.)")
  return()
endif()

set(sources
  detector.cpp
)

add_executable(detector ${sources})

include_directories(${MARSYAS_INCLUDE_DIR})
target_link_libraries(detector ${MARSYAS_LIB})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/detector.mrs detector.mrs COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/delta_ratio.mrs delta_ratio.mrs COPYONLY)

set(scripts
  ${CMAKE_CURRENT_SOURCE_DIR}/onset_function.mrs
  ${CMAKE_CURRENT_SOURCE_DIR}/onsets.mrs
  ${CMAKE_CURRENT_SOURCE_DIR}/onset_features.mrs
  ${CMAKE_CURRENT_SOURCE_DIR}/onset_features2.mrs
  ${CMAKE_CURRENT_SOURCE_DIR}/delta_ratio.mrs
)

install(
  FILES ${scripts}
  DESTINATION "/home/jakob/.config/marsyas/scripts"
)
