cmake_minimum_required(VERSION 3.18)

project(
  jpegloader
  VERSION 0.1.0.4
  DESCRIPTION "Lightweight JPEG loader library"
  HOMEPAGE_URL "https://gitverse.ru/DimaO87/jpegloader"
  LANGUAGES C
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(
  SOURCES
  src/bitstream.c
  src/idct.c
  src/jpegapp.c
  src/jpegcom.c
  src/jpegdht.c
  src/jpegdqt.c
  src/jpegfunc.c
  src/jpegloader.c
  src/jpegparser.c
  src/jpegsof.c
  src/jpegsos.c
  src/huffman.c
)

set(
  PRIVATE_HEADERS
  src/bitstream.h
  src/idct.h
  src/jpegapp.h
  src/jpegcom.h
  src/jpegcontext.h
  src/jpegdht.h
  src/jpegdqt.h
  src/jpegfunc.h
  src/jpeglog.h
  src/jpegparser.h
  src/jpegsof.h
  src/jpegsos.h
  src/huffman.h
)

set(
  PUBLIC_HEADERS
  include/jpegloader/jpegloader.h
  include/jpegloader/jpegtypes.h
)

# Endianness
include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if (IS_BIG_ENDIAN)
  set(WORDS_BIGENDIAN 1)
else()
  unset(WORDS_BIGENDIAN)
endif()

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
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
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
option(BUILD_DOCS "Enable documentation building" ON)
if (BUILD_DOCS)
find_program(DOXYGEN doxygen)

# find_program(
#   MAKEINFO
#   makeinfo
# )
add_subdirectory(doc)
endif()

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
option(DISABLE_TESTING "Disable test routine" OFF)
if(NOT DISABLE_TESTING)
  add_subdirectory(tests)
else()
  message(NOTICE "Testing is disabled. ``check'' is unavailable")
endif()
