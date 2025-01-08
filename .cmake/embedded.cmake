# This cmake script is used to generate a single header file
find_package(Python 3.6 QUIET)

if (NOT PYTHON_FOUND)
  message(STATUS "Python 3.6 not found. Single-header include file will NOT be created")
  return()
endif()

find_program(QUOM_EXECUTABLE NAMES quom)
if (QUOM_EXECUTABLE-NOTFOUND)
  message(STATUS "quom program not found. Install it with pip or easy_install")
  return()
endif()

# add_custom_target(single-header
#   COMMAND
#     ${QUOM_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/include/baklaga/http.hpp ${CMAKE_CURRENT_SOURCE_DIR}/single-header/baklaga.hpp --include_directory "${CMAKE_CURRENT_SOURCE_DIR}/include"
#     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#   COMMENT 
#     "Generating single header file"
# )

execute_process(
    COMMAND ${QUOM_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/include/baklaga/http.hpp ${CMAKE_CURRENT_SOURCE_DIR}/single-header/baklaga.hpp --include_directory "${CMAKE_CURRENT_SOURCE_DIR}/include"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    OUTPUT_VARIABLE QUOM_OUTPUT
    ERROR_VARIABLE QUOM_ERROR
    RESULT_VARIABLE QUOM_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
)
