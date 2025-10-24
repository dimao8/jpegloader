cmake_minimum_required(VERSION 3.18)

project(
  jpegloader
  VERSION 0.1.0.0
  DESCRIPTION "Lite JPEG loader library"
  HOMEPAGE_URL "https://gitverse.ru/DimaO87/jpegloader"
  LANGUAGES C
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(
  SOURCES
  src/jpegloader.c
)

set(
  HEADERS
  include/jpegloader/jpegloader.h
)

# Dependencies

# Library
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(
  jpegloader PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
  $<INSTALL_INTERFACE:include/jpegloader>
)
set_target_properties(
  ${PROJECT_NAME} PROPERTIES
  PUBLIC_HEADER "${HEADERS}"
)
# add_dependencies(
#   jpegloader pdf
# )
target_compile_definitions(
  jpegloader PUBLIC
  "-DHAVE_CONFIG_H"
)
target_link_libraries(
  jpegloader PUBLIC
  ${ZLIB_LIBRARIES}
)

# Documentation
find_program(
  MAKEINFO
  makeinfo
)
# if(MAKEINFO)
#   add_subdirectory(doc)
# else()
#   message(WARNING "makeinfo not found. Skip document building.")
# endif()

# Config file
set(PACKAGE ${PROJECT_NAME})
set(VERSION ${PROJECT_VERSION})
configure_file(
  ${CMAKE_SOURCE_DIR}/config.h.in
  ${CMAKE_BINARY_DIR}/config.h
  @ONLY
)

# Configuration
set(INCLUDE_INSTALL_DIR include/ CACHE PATH "Headers install directory")
set(LIB_INSTALL_DIR lib/ CACHE PATH "Libraries install directory")
set(SYSCONFIG_INSTALL_DIR etc/jpegloader/ CACHE PATH "System Configuration install directory")
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/jpegloaderConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/jpegloaderConfig.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/jpegloader/cmake
  PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/jpegloaderConfigVersion.cmake
  VERSION 0.1.0
  COMPATIBILITY SameMajorVersion
)

install(
  TARGETS jpegloader
  EXPORT jpegloader-targets
  PUBLIC_HEADER DESTINATION include/jpegloader
)

install(
  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/jpegloaderConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/jpegloaderConfigVersion.cmake
  DESTINATION ${LIB_INSTALL_DIR}/cmake/jpegloader
)

# if(MAKEINFO)
#   install(
#     FILES ${CMAKE_BINARY_DIR}/doc/jpegloader.pdf
#     TYPE DOC
#   )
# endif()

# Testing
# option(DISABLE_TESTING "Disable test routine" OFF)
# if(NOT DISABLE_TESTING)
#   add_subdirectory(tests)
# else()
#   message(NOTICE "Testing is disabled. ``check'' is unavailable")
# endif()
