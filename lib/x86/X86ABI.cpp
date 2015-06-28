#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Type.hpp>

#include <llvm-abi/x86/X86ABI.hpp>
#include <llvm-abi/x86/X86ABITypeInfo.hpp>

namespace llvm_abi {
	
	namespace x86 {
		
		X86ABI::X86ABI(llvm::Module* module)
		: llvmContext_(module->getContext()),
		typeInfo_(llvmContext_) { }
		
		X86ABI::~X86ABI() { }
		
		std::string X86ABI::name() const {
			return "x86";
		}
		
		const ABITypeInfo& X86ABI::typeInfo() const {
			return typeInfo_;
		}
		
		llvm::CallingConv::ID X86ABI::getCallingConvention(const CallingConvention callingConvention) const {
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
		
		llvm::FunctionType* X86ABI::getFunctionType(const FunctionType& /*functionType*/) const {
			llvm_unreachable("TODO");
		}
		
		llvm::AttributeSet X86ABI::getAttributes(const FunctionType& /*functionType*/,
		                                          const llvm::AttributeSet /*existingAttributes*/) const {
			llvm_unreachable("TODO");
		}
		
		llvm::Value* X86ABI::createCall(Builder& /*builder*/,
		                                 const FunctionType& /*functionType*/,
		                                 std::function<llvm::Value* (llvm::ArrayRef<llvm::Value*>)> /*callBuilder*/,
		                                 llvm::ArrayRef<llvm::Value*> /*arguments*/) const {
			llvm_unreachable("TODO");
		}
		
		std::unique_ptr<FunctionEncoder>
		X86ABI::createFunctionEncoder(Builder& /*builder*/,
		                               const FunctionType& /*functionType*/,
		                               llvm::ArrayRef<llvm::Value*> /*arguments*/) const {
			llvm_unreachable("TODO");
		}
		
	}
	
}

