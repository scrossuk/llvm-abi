#ifndef TESTSYSTEM_HPP
#define TESTSYSTEM_HPP

#include <fstream>
#include <memory>
#include <stdexcept>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_os_ostream.h>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Builder.hpp>
#include <llvm-abi/FunctionEncoder.hpp>
#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

using namespace llvm_abi;

static const char* const X86_64_TRIPLE = "x86_64-none-linux-gnu";

class TestBuilder: public Builder {
public:
	TestBuilder(llvm::Function& function)
	: function_(function),
	builder_(&(function.getEntryBlock())) { }
	
	IRBuilder& getEntryBuilder() {
		if (!function_.getEntryBlock().empty()) {
			builder_.SetInsertPoint(function_.getEntryBlock().begin());
		}
		return builder_;
	}
	
	IRBuilder& getBuilder() {
		builder_.SetInsertPoint(&(function_.getEntryBlock()));
		return builder_;
	}
	
private:
	llvm::Function& function_;
	IRBuilder builder_;
	
};

class TestSystem {
public:
	TestSystem(const std::string& triple,
	           const std::string& cpu)
	: context_(),
	module_("", context_),
	abi_(createABI(module_, llvm::Triple(triple), cpu)),
	nextIntegerValue_(1) { }
	
	ABI& abi() {
		return *abi_;
	}
	
	llvm::Constant* getConstantValue(const Type type) {
		const auto& typeInfo = abi_->typeInfo();
		switch (type.kind()) {
			case VoidType:
				return llvm::UndefValue::get(llvm::Type::getVoidTy(context_));
			case PointerType:
				return llvm::ConstantPointerNull::get(llvm::Type::getInt8PtrTy(context_));
			case UnspecifiedWidthIntegerType:
			case FixedWidthIntegerType:
				return llvm::ConstantInt::get(typeInfo.getLLVMType(type),
				                              nextIntegerValue_++);
			case FloatingPointType:
				return llvm::ConstantFP::get(typeInfo.getLLVMType(type),
				                             static_cast<double>(nextIntegerValue_++));
			case ComplexType:
				llvm_unreachable("TODO");
			case StructType: {
				llvm::SmallVector<llvm::Type*, 8> types;
				llvm::SmallVector<llvm::Constant*, 8> values;
				for (const auto& structMember: type.structMembers()) {
					types.push_back(typeInfo.getLLVMType(structMember.type()));
					values.push_back(getConstantValue(structMember.type()));
				}
				
				const auto structType = llvm::StructType::get(context_, types);
				return llvm::ConstantStruct::get(structType, values);
			}
			case ArrayType: {
				llvm::SmallVector<llvm::Constant*, 8> values;
				for (size_t i = 0; i < type.arrayElementCount(); i++) {
					values.push_back(getConstantValue(type.arrayElementType()));
				}
				
				const auto arrayType = llvm::ArrayType::get(typeInfo.getLLVMType(type.arrayElementType()),
				                                            type.arrayElementCount());
				return llvm::ConstantArray::get(arrayType, values);
			}
			case VectorType: {
				llvm::SmallVector<llvm::Constant*, 8> values;
				for (size_t i = 0; i < type.vectorElementCount(); i++) {
					values.push_back(getConstantValue(type.vectorElementType()));
				}
				
				return llvm::ConstantVector::get(values);
			}
		}
	}
	
	FunctionType makeCallerFunctionType(const TestFunctionType& testFunctionType) const {
		const auto& functionType = testFunctionType.functionType;
		if (!functionType.isVarArg()) {
			return functionType;
		}
		
		const auto& varArgsTypes = testFunctionType.varArgsTypes;
		
		llvm::SmallVector<Type, 8> argumentTypes;
		argumentTypes.reserve(functionType.argumentTypes().size() + varArgsTypes.size());
		
		for (const auto& argType: functionType.argumentTypes()) {
			argumentTypes.push_back(argType);
		}
		
		for (const auto& argType: varArgsTypes) {
			argumentTypes.push_back(argType);
		}
		
		return FunctionType(functionType.returnType(),
		                    argumentTypes,
		                    /*isVarArg=*/false);
	}
	
	void doTest(const std::string& testName, const TestFunctionType& testFunctionType) {
		const auto& calleeFunctionType = testFunctionType.functionType;
		const auto calleeFunction = llvm::cast<llvm::Function>(module_.getOrInsertFunction("callee", abi_->getFunctionType(calleeFunctionType)));
		const auto calleeAttributes = abi_->getAttributes(calleeFunctionType);
		calleeFunction->setAttributes(calleeAttributes);
		
		const auto callerFunctionType = makeCallerFunctionType(testFunctionType);
		const auto callerFunction = llvm::cast<llvm::Function>(module_.getOrInsertFunction("caller", abi_->getFunctionType(callerFunctionType)));
		const auto callerAttributes = abi_->getAttributes(callerFunctionType);
		callerFunction->setAttributes(callerAttributes);
		
		const auto entryBasicBlock = llvm::BasicBlock::Create(context_, "", callerFunction);
		(void) entryBasicBlock;
		
		TestBuilder builder(*callerFunction);
		
		llvm::SmallVector<llvm::Value*, 8> encodedArgumentValues;
		for (auto it = callerFunction->arg_begin();
		     it != callerFunction->arg_end(); ++it) {
			encodedArgumentValues.push_back(it);
		}
		
		auto functionEncoder = abi_->createFunctionEncoder(builder,
		                                                   callerFunctionType,
		                                                   encodedArgumentValues);
		
		llvm::SmallVector<TypedValue, 8> arguments;
		
		for (size_t i = 0; i < functionEncoder->arguments().size(); i++) {
			const auto argValue = functionEncoder->arguments()[i];
			const auto argType = callerFunctionType.argumentTypes()[i];
			arguments.push_back(TypedValue(argValue, argType));
		}
		
		const auto returnValue = abi_->createCall(
			builder,
			calleeFunctionType,
			[&](llvm::ArrayRef<llvm::Value*> values) -> llvm::Value* {
				const auto callInst = builder.getBuilder().CreateCall(calleeFunction, values);
				callInst->setAttributes(calleeAttributes);
				return callInst;
			},
			arguments
		);
		
		functionEncoder->returnValue(returnValue);
		
		std::string filename;
		filename += "test-";
		filename += abi_->name();
		filename += "-" + testName;
		filename += ".output.ll";
		
		std::ofstream file(filename.c_str());
		file << "; Callee function type: " << std::endl;
		file << "; " << calleeFunctionType.toString() << std::endl;
		file << "; Caller function type: " << std::endl;
		file << "; " << callerFunctionType.toString() << std::endl;
		
		llvm::raw_os_ostream ostream(file);
		ostream << module_;
	}
	
private:
	llvm::LLVMContext context_;
	llvm::Module module_;
	std::unique_ptr<ABI> abi_;
	uint64_t nextIntegerValue_;
	
};

#endif