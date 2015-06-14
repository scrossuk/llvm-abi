#ifndef LLVMABI_FUNCTIONTYPE_HPP
#define LLVMABI_FUNCTIONTYPE_HPP

#include <llvm/ADT/ArrayRef.h>

namespace llvm_abi {
	
	class Type;
	
	/**
	 * \brief Function Type
	 */
	class FunctionType {
	public:
		FunctionType(const Type* pReturnType,
		             llvm::ArrayRef<const Type*> pArgumentTypes,
		             bool pIsVarArg = false)
		: isVarArg_(pIsVarArg),
		returnType_(pReturnType),
		argumentTypes_(pArgumentTypes.begin(), pArgumentTypes.end()) { }
		
		bool isVarArg() const {
			return isVarArg_;
		}
		
		const Type* returnType() const {
			return returnType_;
		}
		
		llvm::ArrayRef<const Type*> argumentTypes() const {
			return argumentTypes_;
		}
		
	private:
		bool isVarArg_;
		const Type* returnType_;
		llvm::SmallVector<const Type*, 8> argumentTypes_;
		
	};
	
}

#endif
