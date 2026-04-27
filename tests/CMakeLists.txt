add_executable(banking_tests
    test_account.cpp
    test_transaction.cpp
)

target_link_libraries(banking_tests
    banking
    gtest_main
    gmock
)

target_include_directories(banking_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../banking
)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(banking_tests PRIVATE -O0 -g --coverage)
        target_link_options(banking_tests PRIVATE --coverage)
    endif()
endif()

include(GoogleTest)
gtest_discover_tests(banking_tests)
