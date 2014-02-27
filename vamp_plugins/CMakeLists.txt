
set( sources
  vamp_marsyas_plugin.cpp
)

add_library( marsyas_vamp_plugin MODULE ${sources} )

target_link_libraries( marsyas_vamp_plugin vamp-sdk marsyas )

set_target_properties( marsyas_vamp_plugin PROPERTIES
  OUTPUT_NAME marsyasvampplugin
  PREFIX ""
)

if(CMAKE_SYSTEM_NAME MATCHES Linux)
  install( TARGETS marsyas_vamp_plugin DESTINATION "lib/vamp" )
endif()
