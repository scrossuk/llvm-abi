#ifndef LLVMABI_X86_64_CLASSIFICATION_HPP
#define LLVMABI_X86_64_CLASSIFICATION_HPP

#include <llvm-abi/x86/ArgClass.hpp>

namespace llvm_abi {
	
	class ABITypeInfo;
	
	namespace x86 {
		
		class Classification {
		public:
			Classification();
			
			ArgClass low() const;
			
			ArgClass high() const;
			
			bool isMemory() const;
			
			void addField(size_t offset, ArgClass fieldClass);
			
			void classifyType(const ABITypeInfo& typeInfo,
			                  Type type,
			                  size_t offset);
			
		private:
			// One class for each eightbyte.
			ArgClass classes_[2];
			
		};
		
	}
	
}

#endif
