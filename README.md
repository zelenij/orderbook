# README

## Building and testing

* Requires a recent C++ compiler
* Google Test (for testing)
  * For Ubuntu install these packages: libgtest-dev googletest
* CMake
* Tested on Ubuntu only, but should work out of the box on any recent Mac or other Linux distribution as well

To compile: invoke cmake, pointing to the top level CMakeLists.txt file's location, for example

cmake <path-to-dir>
make

In the build directory, the resulting binaries are in 

* src/order_book
* tests/order_book_test

To test, run ctest or make test after compiling. ctest -V for more details. You can also invoke the built test artifact, tests/order_book_test

## Considerations
* The implementation is not thread safe

* I use Google unit test framework to validate the implementation.  It's integrated into the cmake build, so it's easy to invoke
