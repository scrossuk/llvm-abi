#ifndef LLVMABI_X86_64_CLASSIFICATION_HPP
#define LLVMABI_X86_64_CLASSIFICATION_HPP

#include "ArgClass.hpp"

namespace llvm_abi {
	
	namespace x86_64 {
		
		class Classification {
		public:
			Classification();
			
			bool isMemory() const;
			
			void addField(size_t offset, ArgClass fieldClass);
			
			void classifyType(Type type, size_t offset);
			
			ArgClass classes[2];
			
		};
		
		Classification classify(Type type);
		
	}
	
}

#endif
