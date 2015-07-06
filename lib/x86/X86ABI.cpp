#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Builder.hpp>
#include <llvm-abi/Callee.hpp>
#include <llvm-abi/Caller.hpp>
#include <llvm-abi/FunctionEncoder.hpp>
#include <llvm-abi/FunctionIRMapping.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypePromoter.hpp>

#include <llvm-abi/x86/X86_32Classifier.hpp>
#include <llvm-abi/x86/X86ABI.hpp>
#include <llvm-abi/x86/X86ABITypeInfo.hpp>

namespace llvm_abi {
	
	namespace x86 {
		
		X86ABI::X86ABI(llvm::Module* module)
		: llvmContext_(module->getContext()),
		typeInfo_(llvmContext_) { }
		
		X86ABI::~X86ABI() { }
		
		std::string X86ABI::name() const {
			return "x86";
		}
		
		const ABITypeInfo& X86ABI::typeInfo() const {
			return typeInfo_;
		}
		
		llvm::CallingConv::ID X86ABI::getCallingConvention(const CallingConvention callingConvention) const {
			switch (callingConvention) {
				case CC_CDefault:
				case CC_CDecl:
				case CC_CppDefault:
					return llvm::CallingConv::C;
				case CC_StdCall:
					return llvm::CallingConv::X86_StdCall;
				case CC_FastCall:
					return llvm::CallingConv::X86_FastCall;
				case CC_ThisCall:
					return llvm::CallingConv::X86_ThisCall;
				case CC_Pascal:
					return llvm::CallingConv::X86_StdCall;
				default:
					llvm_unreachable("Invalid calling convention for ABI.");
			}
		}
		
		llvm::FunctionType* X86ABI::getFunctionType(const FunctionType& functionType) const {
			X86_32Classifier classifier(typeInfo_);
			const auto argInfoArray =
				classifier.classifyFunctionType(functionType,
				                                functionType.argumentTypes(),
				                                llvm::CallingConv::C);
			assert(argInfoArray.size() >= 1);
			
			const auto functionIRMapping = getFunctionIRMapping(argInfoArray);
			
			return llvm_abi::getFunctionType(llvmContext_,
			                                 typeInfo_,
			                                 functionType,
			                                 functionIRMapping);
		}
		
		llvm::AttributeSet X86ABI::getAttributes(const FunctionType& functionType,
		                                         llvm::ArrayRef<Type> rawArgumentTypes,
		                                         const llvm::AttributeSet existingAttributes) const {
			assert(rawArgumentTypes.size() >= functionType.argumentTypes().size());
			
			// Promote argument types (e.g. for varargs).
			TypePromoter typePromoter(typeInfo());
			const auto argumentTypes = typePromoter.promoteArgumentTypes(functionType,
			                                                             rawArgumentTypes);
			
			X86_32Classifier classifier(typeInfo_);
			const auto argInfoArray =
				classifier.classifyFunctionType(functionType,
				                                argumentTypes,
				                                llvm::CallingConv::C);
			assert(argInfoArray.size() >= 1);
			
			const auto functionIRMapping = getFunctionIRMapping(argInfoArray);
			
			return llvm_abi::getFunctionAttributes(llvmContext_,
			                                       typeInfo_,
			                                       functionIRMapping,
			                                       existingAttributes);
		}
		
		llvm::Value* X86ABI::createCall(Builder& builder,
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
			
			X86_32Classifier classifier(typeInfo_);
			const auto argInfoArray =
				classifier.classifyFunctionType(functionType,
				                                argumentTypes,
				                                llvm::CallingConv::C);
			assert(argInfoArray.size() >= 1);
			
			const auto functionIRMapping = getFunctionIRMapping(argInfoArray);
			
			Caller caller(typeInfo_,
			              functionType,
			              functionIRMapping,
			              builder);
			
			const auto encodedArguments = caller.encodeArguments(arguments);
			
			const auto returnValue = callBuilder(encodedArguments);
			
			return caller.decodeReturnValue(encodedArguments, returnValue);
		}
		
		std::unique_ptr<FunctionEncoder>
		X86ABI::createFunctionEncoder(Builder& /*builder*/,
		                               const FunctionType& /*functionType*/,
		                               llvm::ArrayRef<llvm::Value*> /*arguments*/) const {
			llvm_unreachable("TODO");
		}
		
	}
	
}

