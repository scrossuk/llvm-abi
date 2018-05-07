#ifndef LLVMABI_X86_WIN64ABI_HPP
#define LLVMABI_X86_WIN64ABI_HPP

#include <unordered_map>
#include <vector>

#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/CallingConvention.hpp>
#include <llvm-abi/Type.hpp>

namespace llvm_abi {
	
	namespace x86 {
		
		class Win64ABI: public ABI {
		public:
			Win64ABI(llvm::Module* module);
			~Win64ABI();
			
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
			
			std::unique_ptr<FunctionEncoder> createFunctionEncoder(Builder& builder,
			                                                       const FunctionType& functionType,
			                                                       llvm::ArrayRef<llvm::Value*> arguments) const;
			
		private:
			llvm::LLVMContext& llvmContext_;
			
		};
		
	}
	
}

#endif
