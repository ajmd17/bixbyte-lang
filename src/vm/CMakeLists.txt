cmake_minimum_required(VERSION 3.5)

file (GLOB_RECURSE vm_SOURCES "*.c")
file (GLOB_RECURSE vm_HEADERS "../../include/vm/*.hpp")

foreach(f IN LISTS vm_SOURCES)
  get_filename_component(b ${f} NAME)
  set_source_files_properties(${f} PROPERTIES
    COMPILE_DEFINITIONS "__FILENAME__=${b}")
endforeach()
foreach(f IN LISTS vm_HEADERS)
  get_filename_component(b ${f} NAME)
  set_source_files_properties(${f} PROPERTIES
    COMPILE_DEFINITIONS "__FILENAME__=${b}")
endforeach()

add_executable(vm ${vm_SOURCES} ${vm_HEADERS})
