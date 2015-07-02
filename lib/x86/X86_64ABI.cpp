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
		                                            llvm::ArrayRef<Type> argumentTypes,
		                                            const llvm::AttributeSet existingAttributes) const {
			assert(argumentTypes.size() >= functionType.argumentTypes().size());
			
			llvm::SmallVector<llvm::AttributeSet, 8> attributes;
			
			const auto functionIRMapping = computeIRMapping(typeInfo_,
			                                                functionType,
			                                                functionType.argumentTypes());
			
			llvm::AttrBuilder functionAttrs(existingAttributes, llvm::AttributeSet::FunctionIndex);
			llvm::AttrBuilder returnAttrs(existingAttributes, llvm::AttributeSet::ReturnIndex);
			
			const auto returnType = functionType.returnType();
			
			const auto& returnArgInfo = functionIRMapping.returnArgInfo();
			
			switch (returnArgInfo.getKind()) {
				case ArgInfo::ExtendInteger:
					if (returnType.hasSignedIntegerRepresentation(typeInfo_)) {
						returnAttrs.addAttribute(llvm::Attribute::SExt);
					} else if (returnType.hasUnsignedIntegerRepresentation(typeInfo_)) {
						returnAttrs.addAttribute(llvm::Attribute::ZExt);
					}
					// FALL THROUGH
				case ArgInfo::Direct:
					if (returnArgInfo.getInReg()) {
						returnAttrs.addAttribute(llvm::Attribute::InReg);
					}
					break;
				case ArgInfo::Ignore:
					break;
				case ArgInfo::InAlloca:
				case ArgInfo::Indirect: {
					// inalloca and sret disable readnone and readonly
					functionAttrs.removeAttribute(llvm::Attribute::ReadOnly);
					functionAttrs.removeAttribute(llvm::Attribute::ReadNone);
					break;
				}
				case ArgInfo::Expand:
					llvm_unreachable("Invalid ABI kind for return argument");
			}
			
			// Attach return attributes.
			if (returnAttrs.hasAttributes()) {
				attributes.push_back(llvm::AttributeSet::get(context(), llvm::AttributeSet::ReturnIndex, returnAttrs));
			}
			
			// Attach attributes to sret.
			if (functionIRMapping.hasStructRetArg()) {
				llvm::AttrBuilder structRetAttrs;
				structRetAttrs.addAttribute(llvm::Attribute::StructRet);
				structRetAttrs.addAttribute(llvm::Attribute::NoAlias);
				if (returnArgInfo.getInReg()) {
					structRetAttrs.addAttribute(llvm::Attribute::InReg);
				}
				attributes.push_back(llvm::AttributeSet::get(
						context(), functionIRMapping.structRetArgIndex() + 1, structRetAttrs));
			}
			
			// Attach attributes to inalloca argument.
			if (functionIRMapping.hasInallocaArg()) {
				llvm::AttrBuilder attrs;
				attrs.addAttribute(llvm::Attribute::InAlloca);
				attributes.push_back(llvm::AttributeSet::get(context(), functionIRMapping.inallocaArgIndex() + 1, attrs));
			}
			
			for (size_t argIndex = 0; argIndex < functionType.argumentTypes().size(); argIndex++) {
				const auto argumentType = functionType.argumentTypes()[argIndex];
				const auto& argInfo = functionIRMapping.arguments()[argIndex].argInfo;
				
				llvm::AttrBuilder attrs(existingAttributes, argIndex + 1);
				
				// Add attribute for padding argument, if necessary.
				if (functionIRMapping.hasPaddingArg(argIndex) &&
				    argInfo.getPaddingInReg()) {
					attributes.push_back(llvm::AttributeSet::get(
							context(), functionIRMapping.paddingArgIndex(argIndex) + 1,
							llvm::Attribute::InReg));
				}
				
				switch (argInfo.getKind()) {
					case ArgInfo::ExtendInteger:
						if (argumentType.hasSignedIntegerRepresentation(typeInfo_)) {
							attrs.addAttribute(llvm::Attribute::SExt);
						} else if (argumentType.hasUnsignedIntegerRepresentation(typeInfo_)) {
							attrs.addAttribute(llvm::Attribute::ZExt);
						}
						// FALL THROUGH
					case ArgInfo::Direct:
						/*if (argIndex == 0 && FI.isChainCall())
							attrs.addAttribute(llvm::Attribute::Nest);
						else */
						if (argInfo.getInReg()) {
							attrs.addAttribute(llvm::Attribute::InReg);
						}
						break;
					case ArgInfo::Indirect:
						if (argInfo.getInReg()) {
							attrs.addAttribute(llvm::Attribute::InReg);
						}
						
						if (argInfo.getIndirectByVal()) {
							attrs.addAttribute(llvm::Attribute::ByVal);
						}
						
						attrs.addAlignmentAttr(argInfo.getIndirectAlign());
						
						// byval disables readnone and readonly.
						functionAttrs.removeAttribute(llvm::Attribute::ReadOnly);
						functionAttrs.removeAttribute(llvm::Attribute::ReadNone);
						break;
					case ArgInfo::Ignore:
					case ArgInfo::Expand:
						continue;
					case ArgInfo::InAlloca:
						// inalloca disables readnone and readonly.
						functionAttrs.removeAttribute(llvm::Attribute::ReadOnly);
						functionAttrs.removeAttribute(llvm::Attribute::ReadNone);
						continue;
				}
				
				if (attrs.hasAttributes()) {
					unsigned firstIRArg, numIRArgs;
					std::tie(firstIRArg, numIRArgs) = functionIRMapping.getIRArgRange(argIndex);
					for (unsigned i = 0; i < numIRArgs; i++) {
						attributes.push_back(llvm::AttributeSet::get(context(), firstIRArg + i + 1, attrs));
					}
				}
			}
			
			if (functionAttrs.hasAttributes()) {
				attributes.push_back(llvm::AttributeSet::get(context(), llvm::AttributeSet::FunctionIndex, functionAttrs));
			}
			
			return llvm::AttributeSet::get(context(), attributes);
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

