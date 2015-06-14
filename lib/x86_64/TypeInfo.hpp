#ifndef LLVMABI_X86_64_TYPEINFO_HPP
#define LLVMABI_X86_64_TYPEINFO_HPP

namespace llvm_abi {
	
	class Type;
	
	namespace x86_64 {
		
		size_t getTypeAlign(const Type* type);
		
		size_t getTypeSize(const Type* type);
		
	}
	
}

#endif
