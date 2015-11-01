#ifndef LLVMABI_LLVMUTILS_HPP
#define LLVMABI_LLVMUTILS_HPP

#include <llvm/IR/Value.h>

namespace llvm_abi {
	
	class ABITypeInfo;
	class Builder;
	
	llvm::AllocaInst* createTempAlloca(const ABITypeInfo& typeInfo,
	                                   Builder& builder,
	                                   const Type type,
	                                   const llvm::Twine& name = "");
	
	llvm::AllocaInst* createMemTemp(const ABITypeInfo& typeInfo,
	                                Builder& builder,
	                                const Type type,
	                                const llvm::Twine& name = "");
	
	llvm::StoreInst* createStore(llvm::IRBuilder<>& builder,
	                             llvm::Value* const value,
	                             llvm::Value* const ptr);
	
	llvm::Value* createConstGEP2_32(Builder& builder,
	                                llvm::Type* type, llvm::Value* ptr,
	                                unsigned idx0, unsigned idx1,
	                                const llvm::Twine& name = "");
	
	llvm::Value* createStructGEP(Builder& builder, llvm::Type* type,
	                             llvm::Value* ptr, unsigned idx,
	                             const llvm::Twine& name = "");
	
}

#endif