add_library(llvm-abi
	ABI.cpp
	FunctionIRMapping.cpp
	Type.cpp
	TypeBuilder.cpp
	win64/ABI_Win64.cpp
	x86/ABI_x86.cpp
	x86_64/ArgClass.cpp
	x86_64/Classification.cpp
	x86_64/Classifier.cpp
	x86_64/X86_64ABI.cpp
	x86_64/X86_64ABITypeInfo.cpp
)

install(TARGETS llvm-abi
	ARCHIVE DESTINATION "lib"
)
