#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Type.hpp>

#include "ABI_Win64.hpp"

namespace llvm_abi {
	
	ABI_Win64::ABI_Win64(llvm::Module* module)
	: llvmContext_(module->getContext()) { }
	
	ABI_Win64::~ABI_Win64() { }
	
	std::string ABI_Win64::name() const {
		return "Win64";
	}
	
	const ABITypeInfo& ABI_Win64::typeInfo() const {
		llvm_unreachable("TODO");
	}
	
	llvm::CallingConv::ID ABI_Win64::getCallingConvention(const CallingConvention callingConvention) const {
		switch (callingConvention) {
			case CC_CDefault:
			case CC_CppDefault:
				return llvm::CallingConv::C;
			default:
				llvm_unreachable("Invalid calling convention for ABI.");
		}
	}
	
	llvm::FunctionType* ABI_Win64::getFunctionType(const FunctionType& /*functionType*/) const {
		llvm_unreachable("TODO");
	}
	
	llvm::AttributeSet ABI_Win64::getAttributes(const FunctionType& /*functionType*/,
	                                            const llvm::AttributeSet /*existingAttributes*/) const {
		llvm_unreachable("TODO");
	}
	
	llvm::Value* ABI_Win64::createCall(Builder& /*builder*/,
	                                   const FunctionType& /*functionType*/,
	                                   std::function<llvm::Value* (llvm::ArrayRef<llvm::Value*>)> /*callBuilder*/,
	                                   llvm::ArrayRef<llvm::Value*> /*arguments*/) const {
		llvm_unreachable("TODO");
	}
	
	std::unique_ptr<FunctionEncoder> ABI_Win64::createFunctionEncoder(Builder& /*builder*/,
	                                                                  const FunctionType& /*functionType*/,
	                                                                  llvm::ArrayRef<llvm::Value*> /*arguments*/) const {
		llvm_unreachable("TODO");
	}
	
}

