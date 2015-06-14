#ifndef LLVMABI_ABI_X86_64_HPP
#define LLVMABI_ABI_X86_64_HPP

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

	using ABISizeCache = std::unordered_map<const Type*, size_t>;
	using ABITypeCache = std::unordered_map<const Type*, llvm::Type*>;
	
	class ABI_x86_64: public ABI {
	public:
		ABI_x86_64(llvm::Module* module);
		~ABI_x86_64();
		
		llvm::LLVMContext& context() {
			return llvmContext_;
		}
		
		llvm::Value* memcpyIntrinsic() const {
			return memcpyIntrinsic_;
		}
		
		llvm::Type* encodedType(const Type* type) const;
		
		std::string name() const;
		
		size_t typeSize(const Type* type) const;
		
		size_t typeAlign(const Type* type) const;
		
		llvm::Type* abiType(const Type* type) const;
		
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
		mutable ABITypeCache abiTypeCache_;
		mutable ABISizeCache alignOfCache_;
		mutable ABISizeCache sizeOfCache_;
		llvm::Value* memcpyIntrinsic_;
		
	};

}

#endif
