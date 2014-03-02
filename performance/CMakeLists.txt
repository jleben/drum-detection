# The name of our project is "PAA". CMakeLists files in this project can 
# refer to the root source directory of the project as ${PAA_SOURCE_DIR} and 
# to the root binary directory of the project as ${PAA_BINARY_DIR}. 
cmake_minimum_required (VERSION 2.6) 
project (PAA) 

# Recurse into the "src" subdirectory. This does not actually 
# cause another cmake executable to run. The same process will walk through 
# the project's entire directory structure. 
add_subdirectory (src) 

