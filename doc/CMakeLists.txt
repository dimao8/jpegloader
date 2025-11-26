set(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/doxygen/Doxyfile)
set(DOXYGEN_DIRLIST
  ${CMAKE_SOURCE_DIR}/README.md
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/include/jpegloader)
list(JOIN DOXYGEN_DIRLIST " " DOXYGEN_DIRS)
set(README ${CMAKE_SOURCE_DIR}/README.md)

if(DOXYGEN)
set(PACKAGE ${PROJECT_NAME})
set(VERSION ${PROJECT_VERSION})
set(DESCRIPTION ${PROJECT_DESCRIPTION})
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/Doxyfile.in
  ${CMAKE_CURRENT_BINARY_DIR}/doxygen/Doxyfile
  @ONLY
)

add_custom_target(
  html
  doxygen ${CMAKE_CURRENT_BINARY_DIR}/doxygen/Doxyfile
)
endif()