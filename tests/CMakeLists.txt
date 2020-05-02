set(TEST_SOURCES 
    OrderBookTests.cxx
)

set(BOOK_TESTS order_book_test)

enable_testing()
find_package(GTest REQUIRED)
include_directories(${PROJECT_SOURCE_DIR}/src)

# creates the executable
add_executable(${BOOK_TESTS} ${TEST_SOURCES})
# indicates the include paths
#target_include_directories(${BOOK_TESTS})
# indicates the link paths
target_link_libraries(${BOOK_TESTS} GTest::GTest GTest::Main ${BOOK_LIB})

# declares a test with our executable
add_test(NAME ${BOOK_TESTS} COMMAND ${BOOK_TESTS})

