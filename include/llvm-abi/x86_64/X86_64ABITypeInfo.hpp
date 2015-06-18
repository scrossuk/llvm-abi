#ifndef LLVMABI_X86_64_X86_64ABITYPEINFO_HPP
#define LLVMABI_X86_64_X86_64ABITYPEINFO_HPP

#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

namespace llvm_abi {
	
	namespace x86_64 {
		
		class X86_64ABITypeInfo: public ABITypeInfo {
		public:
			X86_64ABITypeInfo(llvm::LLVMContext& llvmContext);
			
			const TypeBuilder& typeBuilder() const;
			
			size_t getTypeSize(Type type) const;
			
			size_t getTypeAlign(Type type) const;
			
			llvm::Type* getLLVMType(const Type type) const;
			
			llvm::SmallVector<size_t, 8> calculateStructOffsets(llvm::ArrayRef<StructMember> structMembers) const;
			
			bool isCharSigned() const;
			
		private:
			llvm::LLVMContext& llvmContext_;
			TypeBuilder typeBuilder_;
			
		};
		
	}

}

#endif
