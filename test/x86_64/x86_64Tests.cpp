#include <memory>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_os_ostream.h>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

std::ostream& operator<<(std::ostream& os, const llvm::Type& type) {
	llvm::raw_os_ostream llvmOs(os);
	type.print(llvmOs);
	return os;
}

#include "gtest/gtest.h"

using namespace llvm_abi;

std::unique_ptr<ABI> createX86_64ABI(llvm::Module& module) {
	return createABI(module, llvm::Triple("x86_64-none-linux-gnu"));
}

static llvm::LLVMContext globalContext;

llvm::FunctionType* getX86_64FnType(const FunctionType& functionType) {
	llvm::Module module("getX86_64FnType", globalContext);
	return createX86_64ABI(module)->getFunctionType(functionType);
}

#include "FunctionType/Pass1Int.cpp"
#include "FunctionType/Pass2Ints.cpp"
#include "FunctionType/PassArrayStructDoubleInt.cpp"
#include "FunctionType/PassCharShortIntLongLongPtr.cpp"
#include "FunctionType/PassStruct1Float.cpp"
#include "FunctionType/PassStruct1Int.cpp"
#include "FunctionType/PassStruct2Floats.cpp"
#include "FunctionType/PassStruct2Ints.cpp"
#include "FunctionType/PassStruct3FloatsReturnFloat.cpp"
#include "FunctionType/PassStruct3Ints.cpp"
#include "FunctionType/PassStruct4Ints.cpp"
#include "FunctionType/PassStruct5Ints.cpp"
#include "FunctionType/PassStructArray1Char3Chars.cpp"
#include "FunctionType/PassStructDoubleInt.cpp"
#include "FunctionType/ReturnChar.cpp"
#include "FunctionType/ReturnDouble.cpp"
#include "FunctionType/ReturnFloat.cpp"
#include "FunctionType/ReturnInt.cpp"
#include "FunctionType/ReturnLongDouble.cpp"
#include "FunctionType/ReturnShort.cpp"
