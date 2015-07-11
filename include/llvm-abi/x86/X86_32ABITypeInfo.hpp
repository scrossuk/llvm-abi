#ifndef LLVMABI_X86_X86_32ABITYPEINFO_HPP
#define LLVMABI_X86_X86_32ABITYPEINFO_HPP

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>

namespace llvm_abi {
	
	namespace x86 {
		
		class X86_32ABITypeInfo: public ABITypeInfo {
		public:
			X86_32ABITypeInfo(llvm::LLVMContext& llvmContext)
			: llvmContext_(llvmContext) { }
			
			const TypeBuilder& typeBuilder() const;
			
			DataSize getTypeRawSize(Type type) const;
			
			DataSize getTypeAllocSize(Type type) const;
			
			DataSize getTypeStoreSize(Type type) const;
			
			DataSize getTypeRequiredAlign(Type type) const;
			
			DataSize getTypePreferredAlign(Type type) const;
			
			llvm::Type* getLLVMType(Type type) const;
			
			llvm::SmallVector<DataSize, 8> calculateStructOffsets(llvm::ArrayRef<StructMember> structMembers) const;
			
			bool isLegalVectorType(Type type) const;
			
			bool isBigEndian() const;
			
			bool isCharSigned() const;
			
			bool isHomogeneousAggregateBaseType(Type type) const;
			
			bool isHomogeneousAggregateSmallEnough(Type base,
			                                       uint64_t members) const;
			
		private:
			llvm::LLVMContext& llvmContext_;
			
		};
		
	}
	
}

#endif
