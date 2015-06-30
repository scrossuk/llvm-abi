#ifndef TESTFUNCTIONTYPE_HPP
#define TESTFUNCTIONTYPE_HPP

#include <llvm/ADT/SmallVector.h>

#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/Type.hpp>

namespace llvm_abi {
	
	struct TestFunctionType {
		// ABI function type.
		FunctionType functionType;
		
		// Types to pass to varargs.
		llvm::SmallVector<Type, 8> varArgsTypes;
		
		TestFunctionType(FunctionType argFunctionType,
		                 llvm::SmallVector<Type, 8> argVarArgsTypes)
		: functionType(argFunctionType),
		varArgsTypes(argVarArgsTypes) { }
	};
	
}

#endif