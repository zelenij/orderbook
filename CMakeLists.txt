cmake_minimum_required (VERSION 3.2)
project (OrderBook
    VERSION 1.0.0)
enable_testing()

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Simple OrderBook implementation")
SET(CPACK_PACKAGE_VENDOR "Andre")

add_definitions(-g -Wall -Werror -Wunused-value -Wunused-variable -Wunused-parameter -std=c++1y -pedantic -Werror=sign-compare)

set(BOOK_LIB order_book1)

add_subdirectory("src")
add_subdirectory("tests")
