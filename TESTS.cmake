set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(TEST_SRC "${CMAKE_CURRENT_SOURCE_DIR}/tests/rtest_ac_fixed_constructor.cpp")
set(AC_TYPES_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include/")

# --- TEST ---
add_executable(
 test
 ${TEST_SRC}
)

target_include_directories(
 test
 PRIVATE
 ${AC_TYPES_INCLUDE}
)

# --- DEBUG MODE ---
add_executable(
 debug_test
 ${TEST_SRC}
)
target_include_directories(
 debug_test
 PRIVATE
 ${AC_TYPES_INCLUDE}
)
target_compile_definitions(debug_test PRIVATE "DEBUG")
target_compile_options(debug_test PRIVATE "-g")

# --- MANUAL MODE ---
add_executable(
 manual_test
 ${TEST_SRC}
)
target_include_directories(
 manual_test
 PRIVATE
 ${AC_TYPES_INCLUDE}
)

target_compile_definitions(manual_test PRIVATE "MANUAL")
target_compile_options(manual_test PRIVATE "-g")
