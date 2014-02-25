
set(sources
  delta_ratio.cpp
  threshold.cpp
  detector.cpp
)

add_executable(detector ${sources})

target_link_libraries(detector marsyas)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/detector.mrs detector.mrs COPYONLY)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/detector.mrs DESTINATION .)
