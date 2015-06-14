#include <memory>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_os_ostream.h>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Context.hpp>
#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/Type.hpp>

std::ostream& operator<<(std::ostream& os, const llvm::Type& type) {
	llvm::raw_os_ostream llvmOs(os);
	type.print(llvmOs);
	return os;
}

#include "gtest/gtest.h"

std::unique_ptr<llvm_abi::ABI> createX86_64ABI(llvm::Module& module) {
	return llvm_abi::createABI(module, llvm::Triple("x86_64-none-linux-gnu"));
}

// Passing ints.
#include "FunctionType/Pass1Int.cpp"
#include "FunctionType/Pass2Ints.cpp"

// Passing structs of floats.
#include "FunctionType/PassStruct1Float.cpp"
#include "FunctionType/PassStruct2Floats.cpp"

// Passing structs of ints.
#include "FunctionType/PassStruct1Int.cpp"
#include "FunctionType/PassStruct2Ints.cpp"
// FIXME: #include "FunctionType/PassStruct3Ints.cpp"
// FIXME: #include "FunctionType/PassStruct4Ints.cpp"
// FIXME: #include "FunctionType/PassStruct5Ints.cpp"
