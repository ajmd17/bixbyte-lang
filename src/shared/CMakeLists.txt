cmake_minimum_required(VERSION 3.5)

file (GLOB_RECURSE shared_SOURCES "*.cpp")
file (GLOB_RECURSE shared_HEADERS "../../include/shared/*.hpp")

foreach(f IN LISTS shared_SOURCES)
  get_filename_component(b ${f} NAME)
  set_source_files_properties(${f} PROPERTIES
    COMPILE_DEFINITIONS "__FILENAME__=${b}")
endforeach()
foreach(f IN LISTS shared_HEADERS)
  get_filename_component(b ${f} NAME)
  set_source_files_properties(${f} PROPERTIES
    COMPILE_DEFINITIONS "__FILENAME__=${b}")
endforeach()

add_library(shared ${shared_SOURCES} ${shared_HEADERS})
