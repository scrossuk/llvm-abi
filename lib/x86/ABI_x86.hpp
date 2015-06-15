#ifndef LLVMABI_ABI_X86_HPP
#define LLVMABI_ABI_X86_HPP

#include <vector>

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/CallingConvention.hpp>
#include <llvm-abi/Type.hpp>

namespace llvm_abi {

	class ABI_x86: public ABI {
	public:
		ABI_x86(llvm::Module* module);
		~ABI_x86();
		
		std::string name() const;
		
		size_t typeSize(Type type) const;
		
		size_t typeAlign(Type type) const;
		
		llvm::Type* abiType(Type type) const;
		
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
		
	};

}

#endif
