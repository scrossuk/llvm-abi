# Tests.
# Run 'make test' or 'ctest' to execute these.

find_package(Threads REQUIRED)

include_directories(SYSTEM
	"${PROJECT_SOURCE_DIR}/utils/googletest/include"
)

add_library(llvm-abi-testmain STATIC
	TestMain.cpp
)

function(add_llvm_abi_test name)
	add_executable(llvm-abi-test-${name}
		${ARGN}
	)
	
	target_link_libraries(llvm-abi-test-${name}
		llvm-abi-testmain
		llvm-abi-util-gtest
		llvm-abi
		${LLVM_LIBRARIES}
		tinfo
		${CMAKE_DL_LIBS}
		${CMAKE_THREAD_LIBS_INIT}
	)
	
	add_test(NAME Test${name} COMMAND llvm-abi-test-${name})
endfunction()

add_subdirectory(x86_64)
