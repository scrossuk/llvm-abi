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
		TypePromoter(const ABITypeInfo& typeInfo);
		
		TypedValue promoteValue(Builder& builder,
		                        TypedValue value,
		                        Type type) const;
		
		Type promoteVarArgsArgumentType(Type value) const;
		
		TypedValue promoteVarArgsArgument(Builder& builder,
		                                  TypedValue value) const;
		
		llvm::SmallVector<Type, 8>
		promoteArgumentTypes(const FunctionType& functionType,
		                     llvm::ArrayRef<Type> argumentTypes) const;
		
		llvm::SmallVector<TypedValue, 8>
		promoteArguments(Builder& builder,
		                 const FunctionType& functionType,
		                 llvm::ArrayRef<TypedValue> arguments) const;
		
	private:
		const ABITypeInfo& typeInfo_;
		
	};
	
}

#endif
