#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Type.hpp>

#include <llvm-abi/x86/ABI_x86.hpp>
#include <llvm-abi/x86/X86ABITypeInfo.hpp>

namespace llvm_abi {
	
	ABI_x86::ABI_x86(llvm::Module* module)
	: llvmContext_(module->getContext()),
	typeInfo_(llvmContext_) { }
	
	ABI_x86::~ABI_x86() { }
	
	std::string ABI_x86::name() const {
		return "x86";
	}
	
	const ABITypeInfo& ABI_x86::typeInfo() const {
		return typeInfo_;
	}
	
	llvm::CallingConv::ID ABI_x86::getCallingConvention(const CallingConvention callingConvention) const {
		switch (callingConvention) {
			case CC_CDefault:
			case CC_CDecl:
			case CC_CppDefault:
				return llvm::CallingConv::C;
			case CC_StdCall:
				return llvm::CallingConv::X86_StdCall;
			case CC_FastCall:
				return llvm::CallingConv::X86_FastCall;
			case CC_ThisCall:
				return llvm::CallingConv::X86_ThisCall;
			case CC_Pascal:
				return llvm::CallingConv::X86_StdCall;
			default:
				llvm_unreachable("Invalid calling convention for ABI.");
		}
	}
	
	llvm::FunctionType* ABI_x86::getFunctionType(const FunctionType& /*functionType*/) const {
		llvm_unreachable("TODO");
	}
	
	llvm::AttributeSet ABI_x86::getAttributes(const FunctionType& /*functionType*/,
	                                          const llvm::AttributeSet /*existingAttributes*/) const {
		llvm_unreachable("TODO");
	}
	
	llvm::Value* ABI_x86::createCall(Builder& /*builder*/,
	                                 const FunctionType& /*functionType*/,
	                                 std::function<llvm::Value* (llvm::ArrayRef<llvm::Value*>)> /*callBuilder*/,
	                                 llvm::ArrayRef<llvm::Value*> /*arguments*/) const {
		llvm_unreachable("TODO");
	}
	
	std::unique_ptr<FunctionEncoder>
	ABI_x86::createFunctionEncoder(Builder& /*builder*/,
	                               const FunctionType& /*functionType*/,
	                               llvm::ArrayRef<llvm::Value*> /*arguments*/) const {
		llvm_unreachable("TODO");
	}
	
}

