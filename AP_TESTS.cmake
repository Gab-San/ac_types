set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(AP_TYPES_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/ap_types/")
set(TEST_SRC "${CMAKE_CURRENT_SOURCE_DIR}/tests/rtest_ac_ap.cpp")
set(AC_TYPES_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# --- TEST ---
add_executable(
 ap_test
 "${TEST_SRC}"
)

target_include_directories(
 ap_test
 PRIVATE
 "${AP_TYPES_HEADERS}"
 "${AC_TYPES_INCLUDE}"
)

# --- DEBUG MODE ---
add_executable(
 debug_ap_test
 "${TEST_SRC}"
)
target_include_directories(
 debug_ap_test
 PRIVATE
 "${AP_TYPES_HEADERS}"
 "${AC_TYPES_INCLUDE}"
)
target_compile_definitions(debug_ap_test PRIVATE "DEBUG")

# --- MANUAL MODE ---
## --- UNSIGNED ---
add_executable(
 manual_ap_test
 "${TEST_SRC}"
)

target_include_directories(
 manual_ap_test
 PRIVATE
 "${AP_TYPES_HEADERS}"
 "${AC_TYPES_INCLUDE}"
)

target_compile_definitions(manual_ap_test PRIVATE "MANUAL")
target_compile_options(manual_ap_test PRIVATE "-g")
