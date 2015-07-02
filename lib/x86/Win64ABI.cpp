#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Type.hpp>

#include <llvm-abi/x86/Win64ABI.hpp>

namespace llvm_abi {
	
	namespace x86 {
		
		Win64ABI::Win64ABI(llvm::Module* module)
		: llvmContext_(module->getContext()) { }
		
		Win64ABI::~Win64ABI() { }
		
		std::string Win64ABI::name() const {
			return "Win64";
		}
		
		const ABITypeInfo& Win64ABI::typeInfo() const {
			llvm_unreachable("TODO");
		}
		
		llvm::CallingConv::ID Win64ABI::getCallingConvention(const CallingConvention callingConvention) const {
			switch (callingConvention) {
				case CC_CDefault:
				case CC_CppDefault:
					return llvm::CallingConv::C;
				default:
					llvm_unreachable("Invalid calling convention for ABI.");
			}
		}
		
		llvm::FunctionType* Win64ABI::getFunctionType(const FunctionType& /*functionType*/) const {
			llvm_unreachable("TODO");
		}
		
		llvm::AttributeSet Win64ABI::getAttributes(const FunctionType& /*functionType*/,
		                                           llvm::ArrayRef<Type> /*argumentTypes*/,
		                                           const llvm::AttributeSet /*existingAttributes*/) const {
			llvm_unreachable("TODO");
		}
		
		llvm::Value* Win64ABI::createCall(Builder& /*builder*/,
		                                   const FunctionType& /*functionType*/,
		                                   std::function<llvm::Value* (llvm::ArrayRef<llvm::Value*>)> /*callBuilder*/,
		                                   llvm::ArrayRef<TypedValue> /*arguments*/) const {
			llvm_unreachable("TODO");
		}
		
		std::unique_ptr<FunctionEncoder> Win64ABI::createFunctionEncoder(Builder& /*builder*/,
		                                                                  const FunctionType& /*functionType*/,
		                                                                  llvm::ArrayRef<llvm::Value*> /*arguments*/) const {
			llvm_unreachable("TODO");
		}
		
	}
	
}

