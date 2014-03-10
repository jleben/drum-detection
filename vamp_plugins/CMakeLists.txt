if(NOT VAMP_SDK_FOUND)
  message(STATUS "Not building Vamp plugin. (Vamp SDK not found.)")
  return()
endif()

# Create Vamp plugin target

set( sources
  vamp_marsyas_plugin.cpp
)

add_library( marsyas_vamp_plugin MODULE ${sources} )

include_directories(${VAMP_SDK_INCLUDE_DIR})
target_link_libraries( marsyas_vamp_plugin ${VAMP_SDK_LIB} marsyas )

set_target_properties( marsyas_vamp_plugin PROPERTIES
  OUTPUT_NAME marsyasvampplugin
  PREFIX ""
)

if(CMAKE_SYSTEM_NAME MATCHES Linux)
  install( TARGETS marsyas_vamp_plugin DESTINATION "lib/vamp" )
endif()
