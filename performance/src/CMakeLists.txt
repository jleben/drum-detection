# Add executable called "paa" that is built from the source files. 
# The extensions are automatically found.
cmake_minimum_required (VERSION 2.6) 
if (UNIX)
	add_definitions("-std=c++0x")
endif()
add_executable (paa main.cpp paa.cpp app.cpp file.cpp midicsv.cpp midifile.cpp csv.cpp map.cpp object.cpp)
