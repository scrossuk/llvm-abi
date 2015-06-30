#ifndef LLVMABI_TYPEPROMOTER_HPP
#define LLVMABI_TYPEPROMOTER_HPP

#include <llvm-abi/TypedValue.hpp>

namespace llvm_abi {
	
	class ABITypeInfo;
	class Builder;
	class FunctionType;
	
	/**
	 * \brief Type Promoter
	 * 
	 * This class handles promoting types as required in cases such as
	 * passing varargs arguments.
	 */
	class TypePromoter {
	public:
		TypePromoter(const ABITypeInfo& typeInfo,
		             Builder& builder);
		
		TypedValue promoteValue(llvm::Value* value, Type type);
		
		TypedValue promoteVarArgsArgument(TypedValue value);
		
		llvm::SmallVector<TypedValue, 8>
		promoteArguments(const FunctionType& functionType,
		                 llvm::ArrayRef<TypedValue> arguments);
		
	private:
		const ABITypeInfo& typeInfo_;
		Builder& builder_;
		
	};
	
}

#endif
