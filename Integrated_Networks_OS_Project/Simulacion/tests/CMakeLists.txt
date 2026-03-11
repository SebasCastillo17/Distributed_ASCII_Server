enable_testing()
find_package(GTest QUIET)

if (GTest_FOUND)
    set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

    add_executable(unit_tests
    # TODO: Files to test
    # TODO: src code
)

target_link_libraries(unit_tests
    GTest::gtest
    GTest::gtest_main
)

target_include_directories(unit_tests PUBLIC ${INCLUDE_DIR})

add_test(AllTests unit_tests)
else()
    message(WARNING "Google Test no encontrado. Las pruebas unitarias serán ignoradas.")
endif()