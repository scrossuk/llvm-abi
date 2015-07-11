#ifndef LLVMABI_TYPEDVALUE_HPP
#define LLVMABI_TYPEDVALUE_HPP

#include <llvm/IR/Value.h>

#include <llvm-abi/Type.hpp>

namespace llvm_abi {
	
	class TypedValue {
	public:
		TypedValue(llvm::Value* const argValue,
		           const Type argType)
		: value_(argValue),
		type_(argType) { }
		
		llvm::Value* llvmValue() const {
			return value_;
		}
		
		Type type() const {
			return type_;
		}
		
	private:
		llvm::Value* value_;
		Type type_;
		
	};
	
}

#endif
