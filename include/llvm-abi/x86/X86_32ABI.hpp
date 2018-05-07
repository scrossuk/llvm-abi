#ifndef LLVMABI_X86_X86_32ABI_HPP
#define LLVMABI_X86_X86_32ABI_HPP

#include <vector>

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/CallingConvention.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <llvm-abi/x86/X86_32ABITypeInfo.hpp>

namespace llvm_abi {
	
	namespace x86 {
		
		class X86_32ABI: public ABI {
		public:
			X86_32ABI(llvm::Module* module,
			       llvm::Triple targetTriple);
			~X86_32ABI();
			
			std::string name() const;
			
			const ABITypeInfo& typeInfo() const;
			
			llvm::CallingConv::ID getCallingConvention(CallingConvention callingConvention) const;
			
			llvm::FunctionType* getFunctionType(const FunctionType& functionType) const;
			
			llvm::AttributeList getAttributes(const FunctionType& functionType,
			                                  llvm::ArrayRef<Type> argumentTypes,
			                                  llvm::AttributeList existingAttributes) const;
			
			llvm::Value* createCall(Builder& builder,
			                        const FunctionType& functionType,
			                        std::function<llvm::Value* (llvm::ArrayRef<llvm::Value*>)> callBuilder,
			                        llvm::ArrayRef<TypedValue> arguments) const;
			
			std::unique_ptr<FunctionEncoder>
			createFunctionEncoder(Builder& builder,
			                      const FunctionType& functionType,
			                      llvm::ArrayRef<llvm::Value*> arguments) const;
			
		private:
			llvm::LLVMContext& llvmContext_;
			llvm::Triple targetTriple_;
			TypeBuilder typeBuilder_;
			X86_32ABITypeInfo typeInfo_;
			
		};
		
	}
	
}

#endif
