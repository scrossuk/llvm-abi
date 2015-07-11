#ifndef LLVMABI_FUNCTIONTYPE_HPP
#define LLVMABI_FUNCTIONTYPE_HPP

#include <initializer_list>
#include <string>

#include <llvm/ADT/ArrayRef.h>

#include <llvm-abi/CallingConvention.hpp>
#include <llvm-abi/Type.hpp>

namespace llvm_abi {
	
	/**
	 * \brief Function Type
	 */
	class FunctionType {
	public:
		FunctionType(const CallingConvention pCallingConvention,
		             const Type pReturnType,
		             llvm::ArrayRef<Type> pArgumentTypes,
		             bool pIsVarArg = false)
		: callingConvention_(pCallingConvention),
		isVarArg_(pIsVarArg),
		returnType_(pReturnType),
		argumentTypes_(pArgumentTypes.begin(), pArgumentTypes.end()) { }
		
		FunctionType(const CallingConvention pCallingConvention,
		             const Type pReturnType,
		             std::initializer_list<Type> pArgumentTypes,
		             bool pIsVarArg = false)
		: callingConvention_(pCallingConvention),
		isVarArg_(pIsVarArg),
		returnType_(pReturnType),
		argumentTypes_(pArgumentTypes.begin(), pArgumentTypes.end()) { }
		
		CallingConvention callingConvention() const {
			return callingConvention_;
		}
		
		bool isVarArg() const {
			return isVarArg_;
		}
		
		Type returnType() const {
			return returnType_;
		}
		
		llvm::ArrayRef<Type> argumentTypes() const {
			return argumentTypes_;
		}
		
		std::string toString() const {
			std::string string;
			string += "FunctionType(callingConvention: ";
			string += callingConventionString(callingConvention());
			string += ", returnType: ";
			string += returnType().toString();
			string += ", argumentTypes: [";
			bool first = true;
			for (const auto& argType: argumentTypes()) {
				if (!first) {
					string += ", ";
				} else {
					first = false;
				}
				string += argType.toString();
			}
			string += "])";
			return string;
		}
		
	private:
		CallingConvention callingConvention_;
		bool isVarArg_;
		Type returnType_;
		llvm::SmallVector<Type, 8> argumentTypes_;
		
	};
	
}

#endif
