#ifndef LLVMABI_X86_64_X86_64ABI_HPP
#define LLVMABI_X86_64_X86_64ABI_HPP

#include <unordered_map>
#include <vector>

#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/CallingConvention.hpp>
#include <llvm-abi/Type.hpp>

#include <llvm-abi/x86/CPUFeatures.hpp>
#include <llvm-abi/x86/CPUKind.hpp>
#include <llvm-abi/x86/X86_64ABITypeInfo.hpp>

namespace llvm_abi {
	
	namespace x86 {
		
		using ABISizeCache = std::unordered_map<Type, size_t>;
		using ABITypeCache = std::unordered_map<Type, llvm::Type*>;
		
		class X86_64ABI: public ABI {
		public:
			X86_64ABI(llvm::Module* module,
			          const llvm::Triple& targetTriple,
			          const std::string& cpuName);
			~X86_64ABI();
			
			llvm::LLVMContext& context() const {
				return llvmContext_;
			}
			
			llvm::Value* memcpyIntrinsic() const {
				return nullptr;
			}
			
			std::string name() const;
			
			const ABITypeInfo& typeInfo() const;
			
			llvm::CallingConv::ID getCallingConvention(CallingConvention callingConvention) const;
			
			llvm::FunctionType* getFunctionType(const FunctionType& functionType) const;
			
			llvm::AttributeSet getAttributes(const FunctionType& functionType,
		                                         llvm::AttributeSet existingAttributes) const;
			
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
			CPUKind cpuKind_;
			CPUFeatures cpuFeatures_;
			llvm::Module* module_;
			X86_64ABITypeInfo typeInfo_;
			mutable ABITypeCache abiTypeCache_;
			mutable ABISizeCache alignOfCache_;
			mutable ABISizeCache sizeOfCache_;
			
		};
		
	}
	
}

#endif
