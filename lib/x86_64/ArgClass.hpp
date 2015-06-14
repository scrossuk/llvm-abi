#ifndef LLVMABI_X86_64_ARGCLASS_HPP
#define LLVMABI_X86_64_ARGCLASS_HPP

namespace llvm_abi {
	
	namespace x86_64 {
		
		/**
		 * \brief x86_64 Argument Class
		 */
		enum ArgClass {
			Integer,
			Sse,
			SseUp,
			X87,
			X87Up,
			ComplexX87,
			NoClass,
			Memory
		};
		
		ArgClass mergeClasses(ArgClass first, ArgClass second);
		
	}
	
}

#endif
