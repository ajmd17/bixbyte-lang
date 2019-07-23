cmake_minimum_required(VERSION 3.5)

file (GLOB_RECURSE bcparse_SOURCES "*.cpp")
file (GLOB_RECURSE bcparse_HEADERS "../../include/bcparse/*.hpp")

foreach(f IN LISTS bcparse_SOURCES)
  get_filename_component(b ${f} NAME)
  set_source_files_properties(${f} PROPERTIES
    COMPILE_DEFINITIONS "__FILENAME__=${b}")
endforeach()
foreach(f IN LISTS bcparse_HEADERS)
  get_filename_component(b ${f} NAME)
  set_source_files_properties(${f} PROPERTIES
    COMPILE_DEFINITIONS "__FILENAME__=${b}")
endforeach()

add_executable(bcparse ${bcparse_SOURCES} ${bcparse_HEADERS})
