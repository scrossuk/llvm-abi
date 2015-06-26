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

#include <llvm-abi/x86_64/X86_64ABITypeInfo.hpp>

namespace llvm_abi {
	
	namespace x86_64 {
		
		using ABISizeCache = std::unordered_map<Type, size_t>;
		using ABITypeCache = std::unordered_map<Type, llvm::Type*>;
		
		class X86_64ABI: public ABI {
		public:
			X86_64ABI(llvm::Module* module);
			~X86_64ABI();
			
			llvm::LLVMContext& context() {
				return llvmContext_;
			}
			
			llvm::Value* memcpyIntrinsic() const {
				return nullptr;
			}
			
			std::string name() const;
			
			const X86_64ABITypeInfo& typeInfo() const {
				return typeInfo_;
			}
			
			size_t typeSize(Type type) const;
			
			size_t typeAlign(Type type) const;
			
			llvm::Type* getLLVMType(Type type) const;
			
			std::vector<size_t> calculateStructOffsets(llvm::ArrayRef<StructMember> structMembers) const;
			
			llvm::Type* longDoubleType() const;
			
			llvm::CallingConv::ID getCallingConvention(CallingConvention callingConvention) const;
			
			llvm::FunctionType* getFunctionType(const FunctionType& functionType) const;
			
			llvm::Value* createCall(Builder& builder,
			                        const FunctionType& functionType,
			                        std::function<llvm::Value* (llvm::ArrayRef<llvm::Value*>)> callBuilder,
			                        llvm::ArrayRef<llvm::Value*> arguments);
			
			std::unique_ptr<FunctionEncoder> createFunction(Builder& builder,
			                                                const FunctionType& functionType,
			                                                llvm::ArrayRef<llvm::Value*> arguments);
			
		private:
			llvm::LLVMContext& llvmContext_;
			llvm::Module* module_;
			X86_64ABITypeInfo typeInfo_;
			mutable ABITypeCache abiTypeCache_;
			mutable ABISizeCache alignOfCache_;
			mutable ABISizeCache sizeOfCache_;
			
		};
		
	}
	
}

#endif
