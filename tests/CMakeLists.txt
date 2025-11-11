enable_testing()

set(TESTS_SRC_DIR ${CMAKE_SOURCE_DIR}/tests)
set(TESTS_BIN_DIR ${CMAKE_BINARY_DIR}/tests)

# test
add_executable(
  test
  ${TESTS_SRC_DIR}/test.c
)
target_include_directories(test PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(
  test PRIVATE
  jpegloader
)

# t_test
add_test(
  NAME t_test
  COMMAND $<TARGET_FILE:test> ${TESTS_SRC_DIR}/lenna.jpg
  WORKING_DIRECTORY ${TESTS_BIN_DIR}
)

add_custom_target( 
  check
  COMMAND ${CMAKE_CTEST_COMMAND}
  DEPENDS test
)
