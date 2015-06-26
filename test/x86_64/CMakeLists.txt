
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

function(add_x86_64_call_test name)
	add_test(NAME "x86_64-${name}" COMMAND ParseTest "${CMAKE_CURRENT_SOURCE_DIR}/${name}.ll" "${CLANG_EXECUTABLE}")
endfunction()
