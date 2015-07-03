# Tests.
# Run 'make test' or 'ctest' to execute these.

find_package(Threads REQUIRED)

add_executable(ParseTest
	CCodeGenerator.cpp
	ParseTest.cpp
)

target_link_libraries(ParseTest
	llvm-abi
	${LLVM_LIBRARIES}
	tinfo
	${CMAKE_DL_LIBS}
	${CMAKE_THREAD_LIBS_INIT}
)


add_subdirectory(x86_64)
