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
	
	size_t ABI_Win64::typeSize(Type /*type*/) const {
		llvm_unreachable("TODO");
	}
	
	size_t ABI_Win64::typeAlign(Type /*type*/) const {
		llvm_unreachable("TODO");
	}
	
	llvm::Type* ABI_Win64::getLLVMType(Type /*type*/) const {
		llvm_unreachable("TODO");
	}
	
	std::vector<size_t> ABI_Win64::calculateStructOffsets(llvm::ArrayRef<StructMember> /*structMembers*/) const {
		llvm_unreachable("TODO");
	}
	
	llvm::Type* ABI_Win64::longDoubleType() const {
		return llvm::Type::getX86_FP80Ty(llvmContext_);
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
	                                   llvm::ArrayRef<llvm::Value*> /*arguments*/) {
		llvm_unreachable("TODO");
	}
	
	std::unique_ptr<FunctionEncoder> ABI_Win64::createFunction(Builder& /*builder*/,
	                                                           const FunctionType& /*functionType*/,
	                                                           llvm::ArrayRef<llvm::Value*> /*arguments*/) {
		llvm_unreachable("TODO");
	}
	
}

