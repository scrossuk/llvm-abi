add_library(llvm-abi
	ABI.cpp
	Type.cpp
	TypeBuilder.cpp
	win64/ABI_Win64.cpp
	x86/ABI_x86.cpp
	x86_64/ABI_x86_64.cpp
	x86_64/ArgClass.cpp
	x86_64/Classification.cpp
	x86_64/TypeInfo.cpp
)

install(TARGETS llvm-abi
	ARCHIVE DESTINATION "lib"
)
