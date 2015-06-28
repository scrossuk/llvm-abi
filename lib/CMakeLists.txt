add_library(llvm-abi
	ABI.cpp
	Callee.cpp
	Caller.cpp
	FunctionIRMapping.cpp
	Type.cpp
	TypeBuilder.cpp
	x86/ArgClass.cpp
	x86/Classification.cpp
	x86/Classifier.cpp
	x86/CPUFeatures.cpp
	x86/CPUKind.cpp
	x86/Win64ABI.cpp
	x86/X86_64ABI.cpp
	x86/X86_64ABITypeInfo.cpp
	x86/X86ABI.cpp
	x86/X86ABITypeInfo.cpp
)

install(TARGETS llvm-abi
	ARCHIVE DESTINATION "lib"
)
