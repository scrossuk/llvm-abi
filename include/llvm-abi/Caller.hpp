#ifndef LLVMABI_CALLER_HPP
#define LLVMABI_CALLER_HPP

#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/TypedValue.hpp>

namespace llvm_abi {
	
	class ABITypeInfo;
	class Builder;
	class FunctionIRMapping;
	
	/**
	 * \brief Caller
	 * 
	 * Support class for encoding arguments and decoding return values.
	 */
	class Caller {
	public:
		Caller(const ABITypeInfo& typeInfo,
		       const FunctionType& functionType,
		       const FunctionIRMapping& functionIRMapping,
		       Builder& builder);
		
		/**
		 * \brief Encode function arguments.
		 * 
		 * The return value pointer argument allows unnecessary allocas
		 * to be avoided in some cases (by passing this pointer directly
		 * as a struct-ret in function calls).
		 * 
		 * \param arguments Arguments for function call.
		 * \param returnValuePtr Pointer to return value, if any.
		 * \return The ABI-encoded arguments.
		 */
		llvm::SmallVector<llvm::Value*, 8>
		encodeArguments(llvm::ArrayRef<TypedValue> arguments,
		                llvm::Value* returnValuePtr = nullptr);
		
		/**
		 * \brief Decode return value.
		 */
		llvm::Value*
		decodeReturnValue(llvm::ArrayRef<llvm::Value*> encodedArguments,
		                  llvm::Value* encodedReturnValue,
		                  llvm::Value* returnValuePtr = nullptr);
		
	private:
		const ABITypeInfo& typeInfo_;
		FunctionType functionType_;
		const FunctionIRMapping& functionIRMapping_;
		Builder& builder_;
		
	};
	
}

#endif
