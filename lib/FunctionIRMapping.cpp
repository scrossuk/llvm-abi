#include <algorithm>
#include <cstddef>

#include <llvm/IR/Attributes.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/ErrorHandling.h>

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/ArgInfo.hpp>
#include <llvm-abi/FunctionIRMapping.hpp>
#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

namespace llvm_abi {
	
	FunctionIRMapping
	getFunctionIRMapping(llvm::ArrayRef<ArgInfo> argInfoArray) {
		FunctionIRMapping functionIRMapping;
		
		size_t irArgumentNumber = 0;
		bool swapThisWithSRet = false;
		
		functionIRMapping.setReturnArgInfo(argInfoArray[0]);
		
		const auto& returnArgInfo = functionIRMapping.returnArgInfo();
		
		if (returnArgInfo.getKind() == ArgInfo::Indirect) {
			swapThisWithSRet = returnArgInfo.isSRetAfterThis();
			functionIRMapping.setStructRetArgIndex(
				swapThisWithSRet ? 1 : irArgumentNumber++);
		}
		
		for (size_t argumentNumber = 1; argumentNumber < argInfoArray.size(); argumentNumber++) {
			const auto& argInfo = argInfoArray[argumentNumber];
			
			ArgumentIRMapping argumentIRMapping;
			argumentIRMapping.argInfo = argInfo;
			
			if (argInfo.getPaddingType() != VoidTy) {
				argumentIRMapping.paddingArgIndex = irArgumentNumber++;
			}
			
			switch (argInfo.getKind()) {
				case ArgInfo::ExtendInteger:
				case ArgInfo::Direct: {
					// FIXME: handle sseregparm someday...
					const auto coerceType = argInfo.getCoerceToType();
					if (argInfo.isDirect() && argInfo.getCanBeFlattened() && coerceType.isStruct()) {
						argumentIRMapping.numberOfIRArgs = coerceType.structMembers().size();
					} else {
						argumentIRMapping.numberOfIRArgs = 1;
					}
					break;
				}
				case ArgInfo::Indirect:
					argumentIRMapping.numberOfIRArgs = 1;
					break;
				case ArgInfo::Ignore:
				case ArgInfo::InAlloca:
					// ignore and inalloca doesn't have matching LLVM parameters.
					argumentIRMapping.numberOfIRArgs = 0;
					break;
				case ArgInfo::Expand: {
					llvm_unreachable("TODO");
					//argumentIRMapping.numberOfIRArgs = getExpansionSize(ArgType, Context);
					break;
				}
			}
			
			if (argumentIRMapping.numberOfIRArgs > 0) {
				argumentIRMapping.firstArgIndex = irArgumentNumber;
				irArgumentNumber += argumentIRMapping.numberOfIRArgs;
			}
			
			// Skip over the sret parameter when it comes
			// second. We already handled it above.
			if (irArgumentNumber == 1 && swapThisWithSRet) {
				irArgumentNumber++;
			}
			
			functionIRMapping.arguments().push_back(argumentIRMapping);
		}
		
// 		if (FI.usesInAlloca()) {
// 			functionIRMapping.setInallocaArgIndex(irArgumentNumber++);
// 		}
		
		functionIRMapping.setTotalIRArgs(irArgumentNumber);
		
		return functionIRMapping;
	}
	
	llvm::FunctionType *
	getFunctionType(llvm::LLVMContext& context,
	                const ABITypeInfo& typeInfo,
	                const FunctionType& functionType,
	                const FunctionIRMapping& functionIRMapping) {
		llvm::Type* resultType = nullptr;
		
		const auto& returnArgInfo = functionIRMapping.returnArgInfo();
		switch (returnArgInfo.getKind()) {
			case ArgInfo::Expand:
				llvm_unreachable("Invalid ABI kind for return argument");
			
			case ArgInfo::ExtendInteger:
			case ArgInfo::Direct:
				resultType = typeInfo.getLLVMType(returnArgInfo.getCoerceToType());
				break;
			
			case ArgInfo::InAlloca: {
				if (returnArgInfo.getInAllocaSRet()) {
					// sret things on win32 aren't void, they return the sret pointer.
					llvm::Type* const pointeeType = typeInfo.getLLVMType(functionType.returnType());
					const unsigned addressSpace = 0;
					resultType = llvm::PointerType::get(pointeeType, addressSpace);
				} else {
					resultType = llvm::Type::getVoidTy(context);
				}
				break;
			}
			
			case ArgInfo::Indirect: {
				assert(!returnArgInfo.getIndirectAlign() && "Align unused on indirect return.");
				resultType = llvm::Type::getVoidTy(context);
				break;
			}
			
			case ArgInfo::Ignore:
				resultType = llvm::Type::getVoidTy(context);
				break;
		}
		
		llvm::SmallVector<llvm::Type*, 8> argumentTypes(functionIRMapping.totalIRArgs());
		
		// Add type for sret argument.
		if (functionIRMapping.hasStructRetArg()) {
			llvm::Type* const pointeeType = typeInfo.getLLVMType(functionType.returnType());
			const unsigned addressSpace = 0;
			argumentTypes[functionIRMapping.structRetArgIndex()] =
				llvm::PointerType::get(pointeeType, addressSpace);
		}
		
		// Add type for inalloca argument.
		if (functionIRMapping.hasInallocaArg()) {
			llvm_unreachable("TODO");
// 			auto argStruct = FI.getArgStruct();
// 			assert(argStruct);
// 			argumentTypes[functionIRMapping.inallocaArgNo()] = argStruct->getPointerTo();
		}
		
		// Add in all of the required arguments.
		for (size_t argumentNumber = 0;
		     argumentNumber < functionIRMapping.arguments().size();
		     argumentNumber++) {
			const auto& argInfo = functionIRMapping.arguments()[argumentNumber].argInfo;
			const auto argumentType = functionType.argumentTypes()[argumentNumber];
			
			// Insert a padding type to ensure proper alignment.
			if (functionIRMapping.hasPaddingArg(argumentNumber)) {
				argumentTypes[functionIRMapping.paddingArgIndex(argumentNumber)] =
					typeInfo.getLLVMType(argInfo.getPaddingType());
			}
			
			size_t firstIRArg, numIRArgs;
			std::tie(firstIRArg, numIRArgs) =
				functionIRMapping.getIRArgRange(argumentNumber);
			
			switch (argInfo.getKind()) {
				case ArgInfo::Ignore:
				case ArgInfo::InAlloca:
					assert(numIRArgs == 0);
					break;
				
				case ArgInfo::Indirect: {
					assert(numIRArgs == 1);
					// Indirect arguments are always on
					// the stack, which is addr space #0.
					argumentTypes[firstIRArg] =
						typeInfo.getLLVMType(argumentType)->getPointerTo();
					break;
				}
				
				case ArgInfo::ExtendInteger:
				case ArgInfo::Direct: {
					// Fast-isel and the optimizer generally like scalar values better than
					// FCAs, so we flatten them if this is safe to do for this argument.
					const auto coerceType = argInfo.getCoerceToType();
					if (coerceType.isStruct() && argInfo.isDirect() && argInfo.getCanBeFlattened()) {
						assert(numIRArgs == coerceType.structMembers().size());
						for (size_t i = 0; i < coerceType.structMembers().size(); i++) {
							argumentTypes[firstIRArg + i] = typeInfo.getLLVMType(coerceType.structMembers()[i].type());
						}
					} else {
						assert(numIRArgs == 1);
						argumentTypes[firstIRArg] = typeInfo.getLLVMType(coerceType);
					}
					break;
				}

				case ArgInfo::Expand:
					llvm_unreachable("TODO");
// 						auto argumentTypesIter = argumentTypes.begin() + FirstIRArg;
// 						getExpandedTypes(it->type, argumentTypesIter);
// 						assert(argumentTypesIter == argumentTypes.begin() + FirstIRArg + numIRArgs);
					break;
			}
		}
		
		return llvm::FunctionType::get(resultType, argumentTypes, functionType.isVarArg());
	}
	
	llvm::AttributeSet
	getFunctionAttributes(llvm::LLVMContext& llvmContext,
	                      const ABITypeInfo& typeInfo,
	                      const FunctionIRMapping& functionIRMapping,
	                      const llvm::AttributeSet existingAttributes) {
		llvm::SmallVector<llvm::AttributeSet, 8> attributes;
		llvm::AttrBuilder functionAttrs(existingAttributes, llvm::AttributeSet::FunctionIndex);
		llvm::AttrBuilder returnAttrs(existingAttributes, llvm::AttributeSet::ReturnIndex);
		
		const auto& returnArgInfo = functionIRMapping.returnArgInfo();
		
		switch (returnArgInfo.getKind()) {
			case ArgInfo::ExtendInteger: {
				const auto coerceType = returnArgInfo.getCoerceToType();
				if (coerceType.hasSignedIntegerRepresentation(typeInfo)) {
					returnAttrs.addAttribute(llvm::Attribute::SExt);
				} else if (coerceType.hasUnsignedIntegerRepresentation(typeInfo)) {
					returnAttrs.addAttribute(llvm::Attribute::ZExt);
				}
				// FALL THROUGH
			}
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
			attributes.push_back(llvm::AttributeSet::get(llvmContext, llvm::AttributeSet::ReturnIndex, returnAttrs));
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
					llvmContext, functionIRMapping.structRetArgIndex() + 1, structRetAttrs));
		}
		
		// Attach attributes to inalloca argument.
		if (functionIRMapping.hasInallocaArg()) {
			llvm::AttrBuilder attrs;
#if LLVMABI_LLVM_VERSION >= 305
			// InAlloca support was added in LLVM 3.5.
			attrs.addAttribute(llvm::Attribute::InAlloca);
#endif
			attributes.push_back(llvm::AttributeSet::get(llvmContext, functionIRMapping.inallocaArgIndex() + 1, attrs));
		}
		
		for (size_t argIndex = 0; argIndex < functionIRMapping.arguments().size(); argIndex++) {
			const auto& argInfo = functionIRMapping.arguments()[argIndex].argInfo;
			
			llvm::AttrBuilder attrs(existingAttributes, argIndex + 1);
			
			// Add attribute for padding argument, if necessary.
			if (functionIRMapping.hasPaddingArg(argIndex) &&
			    argInfo.getPaddingInReg()) {
				attributes.push_back(llvm::AttributeSet::get(
						llvmContext, functionIRMapping.paddingArgIndex(argIndex) + 1,
						llvm::Attribute::InReg));
			}
			
			switch (argInfo.getKind()) {
				case ArgInfo::ExtendInteger: {
					const auto coerceType = argInfo.getCoerceToType();
					if (coerceType.hasSignedIntegerRepresentation(typeInfo)) {
						attrs.addAttribute(llvm::Attribute::SExt);
					} else if (coerceType.hasUnsignedIntegerRepresentation(typeInfo)) {
						attrs.addAttribute(llvm::Attribute::ZExt);
					}
					// FALL THROUGH
				}
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
					attributes.push_back(llvm::AttributeSet::get(llvmContext, firstIRArg + i + 1, attrs));
				}
			}
		}
		
		if (functionAttrs.hasAttributes()) {
			attributes.push_back(llvm::AttributeSet::get(llvmContext, llvm::AttributeSet::FunctionIndex, functionAttrs));
		}
		
		return llvm::AttributeSet::get(llvmContext, attributes);
	}
	
}
