if(NOT MARSYAS_FOUND)
  message(STATUS "Not building detection app. (Marsyas not found.)")
  return()
endif()

set(scripts
  ${CMAKE_CURRENT_SOURCE_DIR}/onset_function.mrs
  ${CMAKE_CURRENT_SOURCE_DIR}/onsets.mrs
  ${CMAKE_CURRENT_SOURCE_DIR}/detector.mrs
)

set(sources
  detector.cpp
)

add_custom_command(
  OUTPUT scripts.cpp
  COMMAND ${MARSYAS_SCRIPT_CPP}
  -n detector
  scripts
  ${scripts}
  DEPENDS ${scripts}
)

list(APPEND sources ${CMAKE_CURRENT_BINARY_DIR}/scripts.cpp)

add_executable(detector ${sources})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${MARSYAS_INCLUDE_DIR})
target_link_libraries(detector ${MARSYAS_LIB})

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/detector.mrs detector.mrs COPYONLY)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/delta_ratio.mrs delta_ratio.mrs COPYONLY)

install(
  FILES ${scripts}
  DESTINATION "${MARSYAS_SCRIPT_DIR}"
)
