
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

add_x86_64_call_test(Pass1Float)
add_x86_64_call_test(Pass1Int)
add_x86_64_call_test(Pass2Floats)
add_x86_64_call_test(Pass2Ints)
add_x86_64_call_test(Pass6IntsAndChar)
add_x86_64_call_test(Pass6IntsAndPtr)
add_x86_64_call_test(Pass8FloatsAndLongDouble)
add_x86_64_call_test(Pass9Floats)
add_x86_64_call_test(PassArrayStructDoubleInt)
add_x86_64_call_test(PassArrayStructDoubleIntInt)
add_x86_64_call_test(PassArrayStructDoubleIntLong)
add_x86_64_call_test(PassCharShortIntLongLongPtr)
add_x86_64_call_test(PassIntStructInt)
add_x86_64_call_test(PassIntStructShortUintInt)
add_x86_64_call_test(PassLongLongArrayAndReturnLongLongArray)
add_x86_64_call_test(PassStruct1Float)
add_x86_64_call_test(PassStruct1Int)
add_x86_64_call_test(PassStruct2Floats)
add_x86_64_call_test(PassStruct2Ints)
add_x86_64_call_test(PassStruct3Ints)
add_x86_64_call_test(PassStruct4Ints)
add_x86_64_call_test(PassStruct5Ints)
add_x86_64_call_test(PassStructArray1Char3Chars)
add_x86_64_call_test(PassStructDoubleInt)
add_x86_64_call_test(PassStructLongDouble)
add_x86_64_call_test(PassStructLongDoubleLongDouble)
add_x86_64_call_test(PassStructLongPtr)
add_x86_64_call_test(PassStructLongPtrAndReturnPtr)
add_x86_64_call_test(ReturnChar)
add_x86_64_call_test(ReturnDouble)
add_x86_64_call_test(ReturnFloat)
add_x86_64_call_test(ReturnInt)
add_x86_64_call_test(ReturnLongDouble)
add_x86_64_call_test(ReturnSChar)
add_x86_64_call_test(ReturnShort)
add_x86_64_call_test(ReturnStruct1Float)
add_x86_64_call_test(ReturnStruct1Int)
add_x86_64_call_test(ReturnStruct2Floats)
add_x86_64_call_test(ReturnStruct2Ints)
add_x86_64_call_test(ReturnStruct3Ints)
add_x86_64_call_test(ReturnStruct4Ints)
add_x86_64_call_test(ReturnStruct5Ints)
add_x86_64_call_test(ReturnStructLongInt)
add_x86_64_call_test(ReturnUChar)
add_x86_64_call_test(ReturnUShort)
