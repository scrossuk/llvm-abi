#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Builder.hpp>
#include <llvm-abi/LLVMUtils.hpp>

namespace llvm_abi {
	
	llvm::AllocaInst* createTempAlloca(const ABITypeInfo& typeInfo,
	                                   Builder& builder,
	                                   const Type type,
	                                   const llvm::Twine& name) {
		const auto allocaInst = builder.getEntryBuilder().CreateAlloca(typeInfo.getLLVMType(type));
		allocaInst->setName(name);
		return allocaInst;
	}
	
	llvm::AllocaInst* createMemTemp(const ABITypeInfo& typeInfo,
	                                Builder& builder,
	                                const Type type,
	                                const llvm::Twine& name) {
		const auto allocaInst = createTempAlloca(typeInfo,
		                                         builder,
		                                         type,
		                                         name);
		allocaInst->setAlignment(typeInfo.getTypeRequiredAlign(type).asBytes());
		return allocaInst;
	}
	
	llvm::StoreInst* createStore(llvm::IRBuilder<>& builder,
	                             llvm::Value* const value,
	                             llvm::Value* const ptr) {
		assert(ptr->getType()->isPointerTy());
		const auto castPtr = builder.CreatePointerCast(ptr,
		                                               value->getType()->getPointerTo());
		return builder.CreateStore(value, castPtr);
	}
	
	llvm::Value* createConstGEP2_32(Builder& builder,
	                                llvm::Type* type, llvm::Value* ptr,
	                                unsigned idx0, unsigned idx1,
	                                const llvm::Twine& name) {
		assert(ptr->getType()->isPointerTy());
#if LLVMABI_LLVM_VERSION >= 307
		return builder.getBuilder().CreateConstGEP2_32(type, ptr, idx0,
		                                               idx1, name);
#else
		(void) type;
		assert(ptr->getType()->getPointerElementType() == type);
		return builder.getBuilder().CreateConstGEP2_32(ptr, idx0, idx1,
		                                               name);
#endif
	}
	
	llvm::Value* createStructGEP(Builder& builder, llvm::Type* type,
	                             llvm::Value* ptr, unsigned idx,
	                             const llvm::Twine& name) {
		assert(ptr->getType()->isPointerTy());
#if LLVMABI_LLVM_VERSION >= 307
		return builder.getBuilder().CreateStructGEP(type, ptr, idx,
		                                            name);
#else
		(void) type;
		assert(ptr->getType()->getPointerElementType() == type);
		return builder.getBuilder().CreateStructGEP(ptr, idx, name);
#endif
	}
	
}
