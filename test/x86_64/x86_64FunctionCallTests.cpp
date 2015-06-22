#include <memory>
#include <stdexcept>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_os_ostream.h>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Builder.hpp>
#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

std::ostream& operator<<(std::ostream& os, const llvm::Type& type) {
	llvm::raw_os_ostream llvmOs(os);
	type.print(llvmOs);
	return os;
}

#include "gtest/gtest.h"

using namespace llvm_abi;

static const char* const X86_64_TRIPLE = "x86_64-none-linux-gnu";

struct EncodedCallInfo {
	llvm::Value* encodedReturnValue;
	llvm::SmallVector<llvm::Value*, 8> encodedArguments;
};

class TestBuilder: public Builder {
public:
	TestBuilder(llvm::Function& function)
	: builder_(&(function.getEntryBlock())) { }
	
	IRBuilder& getEntryBuilder() {
		return builder_;
	}
	
	IRBuilder& getBuilder() {
		return builder_;
	}
	
private:
	IRBuilder builder_;
	
};

class TestSystem {
public:
	TestSystem(const std::string& triple)
	: context_(),
	module_("Test", context_),
	abi_(createABI(module_, llvm::Triple(triple))),
	nextIntegerValue_(1) { }
	
	ABI& abi() {
		return *abi_;
	}
	
	llvm::Value* getUndefValue() {
		return llvm::UndefValue::get(llvm::Type::getVoidTy(context_));
	}
	
	llvm::Value* getIntTestValue() {
		return llvm::ConstantInt::get(llvm::IntegerType::get(context_, 32),
		                              nextIntegerValue_++);
	}
	
	EncodedCallInfo generateCall(const FunctionType& functionType,
	                             llvm::ArrayRef<llvm::Value*> arguments,
	                             llvm::Value* valueToReturn) {
		const auto llvmFunctionType = abi_->getFunctionType(functionType);
		
		const auto function = llvm::cast<llvm::Function>(module_.getOrInsertFunction("test", llvmFunctionType));
		
		const auto entryBasicBlock = llvm::BasicBlock::Create(context_, "entry", function);
		(void) entryBasicBlock;
		
		EncodedCallInfo callInfo;
		
		TestBuilder builder(*function);
		
		bool wasCalled = false;
		
		callInfo.encodedReturnValue = abi_->createCall(
			builder,
			functionType,
			[&](llvm::ArrayRef<llvm::Value*> values) -> llvm::Value* {
				if (wasCalled) {
					throw std::logic_error("createCall() lambda already called!");
				}
				wasCalled = true;
				
				callInfo.encodedArguments.append(values.begin(),
				                                 values.end());
				return valueToReturn;
			},
			arguments
		);
		
		if (!wasCalled) {
			throw std::logic_error("createCall() lambda wasn't called!");
		}
		
		return callInfo;
	}
	
private:
	llvm::LLVMContext context_;
	llvm::Module module_;
	std::unique_ptr<ABI> abi_;
	uint64_t nextIntegerValue_;
	
};

#include "FunctionCall/Pass1Int.cpp"
#include "FunctionCall/Pass2Ints.cpp"
