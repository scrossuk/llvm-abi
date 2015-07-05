# Tests.
# Run 'make test' or 'ctest' to execute these.

find_package(Threads REQUIRED)

add_executable(ParseTest
	CCodeGenerator.cpp
	ParseTest.cpp
	TokenStream.cpp
)

target_link_libraries(ParseTest
	llvm-abi
	${LLVM_LIBRARIES}
	tinfo
	${CMAKE_DL_LIBS}
	${CMAKE_THREAD_LIBS_INIT}
)

# Search for Clang so we can compare against its output.
set(CLANG_BINARY_SEARCH_NAMES
	clang-3.6
	clang-3.5
	clang-3.4
	clang-3.3
	clang
)

find_program(CLANG_EXECUTABLE
	NAMES ${CLANG_BINARY_SEARCH_NAMES}
	DOC "Path to clang executable"
)

add_subdirectory(x86_64)
