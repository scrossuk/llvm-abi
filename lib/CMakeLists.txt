add_library(llvm-abi
	ABI.cpp
	ABI_x86_64.cpp
	Context.cpp
	Type.cpp
)

install(TARGETS llvm-abi
	ARCHIVE DESTINATION "lib"
)
