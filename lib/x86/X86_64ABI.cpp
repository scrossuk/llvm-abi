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
#include <llvm-abi/TypePromoter.hpp>

#include <llvm-abi/x86/ArgClass.hpp>
#include <llvm-abi/x86/Classification.hpp>
#include <llvm-abi/x86/Classifier.hpp>
#include <llvm-abi/x86/CPUKind.hpp>
#include <llvm-abi/x86/X86_64ABI.hpp>
#include <llvm-abi/x86/X86_64ABITypeInfo.hpp>

namespace llvm_abi {
	
	namespace x86 {
		
		X86_64ABI::X86_64ABI(llvm::Module* module,
		                     const llvm::Triple& targetTriple,
		                     const std::string& cpuName)
		: llvmContext_(module->getContext()),
		cpuKind_(getCPUKind(targetTriple,
		                    cpuName)),
		cpuFeatures_(getCPUFeatures(targetTriple,
		                            cpuKind_)),
		module_(module),
		typeInfo_(llvmContext_, cpuFeatures_) {
			(void) module_;
		}
		
		X86_64ABI::~X86_64ABI() { }
		
		std::string X86_64ABI::name() const {
			return "x86_64";
		}
		
		const ABITypeInfo& X86_64ABI::typeInfo() const {
			return typeInfo_;
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
				classifier.classifyFunctionType(functionType,
				                                functionType.argumentTypes());
			assert(argInfoArray.size() >= 1);
			
			const auto functionIRMapping = getFunctionIRMapping(argInfoArray);
			
			return llvm_abi::getFunctionType(llvmContext_,
			                                 typeInfo_,
			                                 functionType,
			                                 functionIRMapping);
		}
		
		static
		FunctionIRMapping computeIRMapping(const ABITypeInfo& typeInfo,
		                                   const FunctionType& functionType,
		                                   llvm::ArrayRef<Type> argumentTypes) {
			Classifier classifier(typeInfo);
			const auto argInfoArray =
				classifier.classifyFunctionType(functionType,
				                                argumentTypes);
			assert(argInfoArray.size() >= 1);
			
			return getFunctionIRMapping(argInfoArray);
		}
		
		llvm::AttributeSet X86_64ABI::getAttributes(const FunctionType& functionType,
		                                            llvm::ArrayRef<Type> rawArgumentTypes,
		                                            const llvm::AttributeSet existingAttributes) const {
			assert(rawArgumentTypes.size() >= functionType.argumentTypes().size());
			
			// Promote argument types (e.g. for varargs).
			TypePromoter typePromoter(typeInfo());
			const auto argumentTypes = typePromoter.promoteArgumentTypes(functionType,
			                                                             rawArgumentTypes);
			
			const auto functionIRMapping = computeIRMapping(typeInfo_,
			                                                functionType,
			                                                argumentTypes);
			
			return llvm_abi::getFunctionAttributes(llvmContext_,
			                                       typeInfo_,
			                                       functionIRMapping,
			                                       existingAttributes);
		}
		
		llvm::Value* X86_64ABI::createCall(Builder& builder,
		                                    const FunctionType& functionType,
		                                    std::function<llvm::Value* (llvm::ArrayRef<llvm::Value*>)> callBuilder,
		                                    llvm::ArrayRef<TypedValue> rawArguments) const {
			
			TypePromoter typePromoter(typeInfo());
			
			// Promote any varargs arguments (that haven't already been
			// promoted). This changes char => int, float => double etc.
			const auto arguments = typePromoter.promoteArguments(builder,
			                                                     functionType,
			                                                     rawArguments);
			
			llvm::SmallVector<Type, 8> argumentTypes;
			for (const auto& value: arguments) {
				argumentTypes.push_back(value.second);
			}
			
			const auto functionIRMapping = computeIRMapping(typeInfo_,
			                                                functionType,
			                                                argumentTypes);
			
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
			FunctionEncoder_x86_64(const X86_64ABI& abi,
			                       Builder& builder,
		                               const FunctionType& functionType,
			                       llvm::ArrayRef<llvm::Value*> pArguments)
			: builder_(builder),
			functionIRMapping_(computeIRMapping(abi.typeInfo(),
			                                    functionType,
			                                    functionType.argumentTypes())),
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
		
		std::unique_ptr<FunctionEncoder>
		X86_64ABI::createFunctionEncoder(Builder& builder,
		                                 const FunctionType& functionType,
		                                 llvm::ArrayRef<llvm::Value*> arguments) const {
			return std::unique_ptr<FunctionEncoder>(new FunctionEncoder_x86_64(*this,
			                                                               builder,
			                                                               functionType,
			                                                               arguments));
		}
		
	}
	
}

