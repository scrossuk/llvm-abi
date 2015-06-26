#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Builder.hpp>
#include <llvm-abi/Callee.hpp>
#include <llvm-abi/Caller.hpp>
#include <llvm-abi/DataSize.hpp>
#include <llvm-abi/FunctionEncoder.hpp>
#include <llvm-abi/FunctionIRMapping.hpp>
#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/Type.hpp>

#include <llvm-abi/x86_64/ArgClass.hpp>
#include <llvm-abi/x86_64/Classification.hpp>
#include <llvm-abi/x86_64/Classifier.hpp>
#include <llvm-abi/x86_64/X86_64ABI.hpp>
#include <llvm-abi/x86_64/X86_64ABITypeInfo.hpp>

namespace llvm_abi {
	
	namespace x86_64 {
		
		X86_64ABI::X86_64ABI(llvm::Module* module)
		: llvmContext_(module->getContext()),
		module_(module),
		typeInfo_(llvmContext_) {
			(void) module_;
		}
		
		X86_64ABI::~X86_64ABI() { }
		
		std::string X86_64ABI::name() const {
			return "x86_64";
		}
		
		size_t X86_64ABI::typeSize(const Type type) const {
			const auto iterator = sizeOfCache_.find(type);
			if (iterator != sizeOfCache_.end()) {
				return iterator->second;
			}
			
			const auto value = typeInfo_.getTypeAllocSize(type).asBytes();
			sizeOfCache_.insert(std::make_pair(type, value));
			return value;
		}
		
		size_t X86_64ABI::typeAlign(const Type type) const {
			const auto iterator = alignOfCache_.find(type);
			if (iterator != alignOfCache_.end()) {
				return iterator->second;
			}
			
			const auto value = typeInfo_.getTypeRequiredAlign(type).asBytes();
			alignOfCache_.insert(std::make_pair(type, value));
			return value;
		}
		
		llvm::Type* X86_64ABI::getLLVMType(const Type type) const {
			return typeInfo_.getLLVMType(type);
		}
		
		std::vector<size_t> X86_64ABI::calculateStructOffsets(llvm::ArrayRef<StructMember> structMembers) const {
			auto result = typeInfo_.calculateStructOffsets(structMembers);
			std::vector<size_t> offsets;
			offsets.reserve(result.size());
			for (const auto& resultValue: result) {
				offsets.push_back(resultValue.asBytes());
			}
			return offsets;
		}
		
		llvm::Type* X86_64ABI::longDoubleType() const {
			return llvm::Type::getX86_FP80Ty(llvmContext_);
		}
		
		llvm::CallingConv::ID X86_64ABI::getCallingConvention(const CallingConvention callingConvention) const {
			switch (callingConvention) {
				case CC_CDefault:
				case CC_CppDefault:
					return llvm::CallingConv::C;
				default:
					llvm_unreachable("Invalid calling convention for ABI.");
			}
		}
		
		llvm::FunctionType* X86_64ABI::getFunctionType(const FunctionType& functionType) const {
			Classifier classifier(typeInfo_);
			const auto argInfoArray =
				classifier.classifyFunctionType(functionType);
			assert(argInfoArray.size() >= 1);
			
			const auto functionIRMapping = getFunctionIRMapping(argInfoArray);
			
			return llvm_abi::getFunctionType(llvmContext_,
			                                 typeInfo_,
			                                 functionType,
			                                 functionIRMapping);
		}
		
		static
		FunctionIRMapping computeIRMapping(const ABITypeInfo& typeInfo,
		                                   const FunctionType& functionType) {
			Classifier classifier(typeInfo);
			const auto argInfoArray =
				classifier.classifyFunctionType(functionType);
			assert(argInfoArray.size() >= 1);
			
			return getFunctionIRMapping(argInfoArray);
		}
		
		llvm::Value* X86_64ABI::createCall(Builder& builder,
		                                    const FunctionType& functionType,
		                                    std::function<llvm::Value* (llvm::ArrayRef<llvm::Value*>)> callBuilder,
		                                    llvm::ArrayRef<llvm::Value*> arguments) {
			const auto functionIRMapping = computeIRMapping(typeInfo_, functionType);
			
			Caller caller(typeInfo_,
			              functionType,
			              functionIRMapping,
			              builder);
			
			const auto encodedArguments = caller.encodeArguments(arguments);
			
			const auto returnValue = callBuilder(encodedArguments);
			
			return caller.decodeReturnValue(encodedArguments, returnValue);
		}
		
		class FunctionEncoder_x86_64: public FunctionEncoder {
		public:
			FunctionEncoder_x86_64(X86_64ABI& abi,
			                       Builder& builder,
		                               const FunctionType& functionType,
			                       llvm::ArrayRef<llvm::Value*> pArguments)
			: builder_(builder),
			functionIRMapping_(computeIRMapping(abi.typeInfo(),
			                                    functionType)),
			callee_(abi.typeInfo(),
			        functionType,
			        functionIRMapping_,
			        builder),
			encodedArguments_(pArguments),
			arguments_(callee_.decodeArguments(pArguments)) { }
			
			llvm::ArrayRef<llvm::Value*> arguments() const {
				return arguments_;
			}
			
			llvm::ReturnInst* returnValue(llvm::Value* const value) {
				const auto encodedReturnValue = callee_.encodeReturnValue(value,
				                                                          encodedArguments_);
				if (encodedReturnValue->getType()->isVoidTy()) {
					return builder_.getBuilder().CreateRetVoid();
				} else {
					return builder_.getBuilder().CreateRet(encodedReturnValue);
				}
			}
			
			llvm::Value* returnValuePointer() const {
				// TODO!
				return nullptr;
			}
			
		private:
			Builder& builder_;
			FunctionIRMapping functionIRMapping_;
			Callee callee_;
			llvm::ArrayRef<llvm::Value*> encodedArguments_;
			llvm::SmallVector<llvm::Value*, 8> arguments_;
			
		};
		
		std::unique_ptr<FunctionEncoder> X86_64ABI::createFunction(Builder& builder,
		                                                            const FunctionType& functionType,
		                                                            llvm::ArrayRef<llvm::Value*> arguments) {
			return std::unique_ptr<FunctionEncoder>(new FunctionEncoder_x86_64(*this,
			                                                               builder,
			                                                               functionType,
			                                                               arguments));
		}
		
	}
	
}

