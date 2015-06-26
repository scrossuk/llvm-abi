#ifndef LLVMABI_CALLEE_HPP
#define LLVMABI_CALLEE_HPP

#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/FunctionType.hpp>

namespace llvm_abi {
	
	class ABITypeInfo;
	class Builder;
	class FunctionIRMapping;
	
	/**
	 * \brief Callee
	 * 
	 * Support class for decoding arguments and encoding return values.
	 */
	class Callee {
	public:
		Callee(const ABITypeInfo& typeInfo,
		       const FunctionType& functionType,
		       const FunctionIRMapping& functionIRMapping,
		       Builder& builder);
		
		/**
		 * \brief Decode function arguments.
		 */
		llvm::SmallVector<llvm::Value*, 8>
		decodeArguments(llvm::ArrayRef<llvm::Value*> encodedArguments);
		
		/**
		 * \brief Encode return value.
		 */
		llvm::Value*
		encodeReturnValue(llvm::Value* returnValue,
		                  llvm::ArrayRef<llvm::Value*> encodedArguments,
		                  llvm::Value* returnValuePtr = nullptr);
		
	private:
		const ABITypeInfo& typeInfo_;
		FunctionType functionType_;
		const FunctionIRMapping& functionIRMapping_;
		Builder& builder_;
		
	};
	
}

#endif
