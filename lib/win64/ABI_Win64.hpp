#ifndef LLVMABI_ABI_WIN64_HPP
#define LLVMABI_ABI_WIN64_HPP

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
	
	class ABI_Win64: public ABI {
	public:
		ABI_Win64(llvm::Module* module);
		~ABI_Win64();
		
		std::string name() const;
		
		const ABITypeInfo& typeInfo() const;
		
		llvm::CallingConv::ID getCallingConvention(CallingConvention callingConvention) const;
		
		llvm::FunctionType* getFunctionType(const FunctionType& functionType) const;
		
		llvm::AttributeSet getAttributes(const FunctionType& functionType,
		                                 llvm::AttributeSet existingAttributes) const;
		
		llvm::Value* createCall(Builder& builder,
		                        const FunctionType& functionType,
		                        std::function<llvm::Value* (llvm::ArrayRef<llvm::Value*>)> callBuilder,
		                        llvm::ArrayRef<llvm::Value*> arguments) const;
		
		std::unique_ptr<FunctionEncoder> createFunction(Builder& builder,
		                                                const FunctionType& functionType,
		                                                llvm::ArrayRef<llvm::Value*> arguments);
		
	private:
		llvm::LLVMContext& llvmContext_;
		
	};
	
}

#endif
