
set(sources
  delta_ratio.cpp
  threshold.cpp
  detector.cpp
)

add_executable(detector ${sources})

target_link_libraries(detector marsyas)

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/detector.mrs detector.mrs COPYONLY)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/onset_function.mrs onset_function.mrs COPYONLY)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/onsets.mrs onsets.mrs COPYONLY)

#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/detector.mrs DESTINATION .)

set(scripts
  ${CMAKE_CURRENT_SOURCE_DIR}/onset_function.mrs
  ${CMAKE_CURRENT_SOURCE_DIR}/onsets.mrs
)

install(
  FILES ${scripts}
  DESTINATION "/home/jakob/.config/marsyas/scripts"
)
