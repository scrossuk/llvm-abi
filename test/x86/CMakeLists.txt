
function(add_x86_call_test name)
	add_test(NAME "x86-${name}" COMMAND ParseTest "${CMAKE_CURRENT_SOURCE_DIR}/${name}.ll" "${CLANG_EXECUTABLE}")
endfunction()

add_x86_call_test(Pass1Float)
add_x86_call_test(Pass1Int)
add_x86_call_test(Pass2Floats)
add_x86_call_test(Pass2Ints)
add_x86_call_test(PassStruct1Float)
add_x86_call_test(PassStruct1Int)
add_x86_call_test(PassStruct2Floats)
add_x86_call_test(PassStruct2Ints)
