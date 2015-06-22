
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Builder.hpp>
#include <llvm-abi/Caller.hpp>
#include <llvm-abi/FunctionIRMapping.hpp>
#include <llvm-abi/FunctionType.hpp>

namespace llvm_abi {
	
	Caller::Caller(const ABITypeInfo& typeInfo,
	               const FunctionType& functionType,
	               const FunctionIRMapping& functionIRMapping,
	               Builder& builder)
	: typeInfo_(typeInfo),
	functionType_(functionType),
	functionIRMapping_(functionIRMapping),
	builder_(builder) { }
	
	llvm::SmallVector<llvm::Value*, 8>
	Caller::encodeArguments(llvm::ArrayRef<llvm::Value*> arguments,
	                        llvm::Value* const returnValuePtr) {
		assert(functionType_.argumentTypes().size() == arguments.size());
		
		llvm::SmallVector<llvm::Value*, 8> irCallArgs(functionIRMapping_.totalIRArgs());
		const auto& returnArgInfo = functionIRMapping_.returnArgInfo();
		
		// If we're using inalloca, insert the allocation after the stack save.
 		llvm::Value* argMemory = nullptr;
// 		if (llvm::StructType* argStruct = callInfo.getArgStruct()) {
// 			llvm::AllocaInst* const allocaInst =
// 				builder_.getEntryBuilder().CreateAlloca(argStruct);
// 			allocaInst->setUsedWithInAlloca(true);
// 			assert(allocaInst->isUsedWithInAlloca() &&
// 			       !allocaInst->isStaticAlloca());
// 			argMemory = allocaInst;
// 			irCallArgs[functionIRMapping_.getInallocaArgNo()] = argMemory;
// 		}
		
		// If the call returns a temporary with struct return, create a
		// temporary alloca to hold the result, unless one is given to us.
		llvm::Value* structRetPtr = nullptr;
		if (returnArgInfo.isIndirect() || returnArgInfo.isInAlloca()) {
			structRetPtr = returnValuePtr;
			if (structRetPtr == nullptr) {
				const auto returnType = typeInfo_.getLLVMType(functionType_.returnType());
				structRetPtr = builder_.getEntryBuilder().CreateAlloca(returnType);
			}
			if (functionIRMapping_.hasStructRetArg()) {
				irCallArgs[functionIRMapping_.structRetArgIndex()] = structRetPtr;
			} else {
				assert(argMemory != nullptr);
				llvm::Value* const address =
					builder_.getBuilder().CreateStructGEP(argMemory, returnArgInfo.getInAllocaFieldIndex());
				builder_.getBuilder().CreateStore(structRetPtr, address);
			}
		}
		
		for (size_t argumentNumber = 0;
		     argumentNumber < arguments.size();
		     argumentNumber++) {
			const auto argumentValue = arguments[argumentNumber];
			const auto& argInfo = functionIRMapping_.arguments()[argumentNumber].argInfo;
			// TODO: add support for values already being in memory.
			const bool isArgumentInMemory = false;
			const auto& argumentType = functionType_.argumentTypes()[argumentNumber];
			
			if (functionIRMapping_.hasPaddingArg(argumentNumber)) {
				irCallArgs[functionIRMapping_.paddingArgIndex(argumentNumber)] =
					llvm::UndefValue::get(typeInfo_.getLLVMType(argInfo.getPaddingType()));
			}
			
			unsigned firstIRArg, numIRArgs;
			std::tie(firstIRArg, numIRArgs) = functionIRMapping_.getIRArgRange(argumentNumber);
			
			switch (argInfo.getKind()) {
				case ArgInfo::InAlloca: {
					assert(numIRArgs == 0);
// 					assert(getTarget().getTriple().getArch() == llvm::Triple::x86);
					if (isArgumentInMemory) {
						// Replace the placeholder with the appropriate argument slot GEP.
						llvm_unreachable("TODO");
					} else {
						// Store the RValue into the argument struct.
						llvm_unreachable("TODO");
					}
					break;
				}

				case ArgInfo::Indirect: {
					assert(numIRArgs == 1);
					if (!isArgumentInMemory) {
						// Make a temporary alloca to pass the argument.
						const auto allocaInst = builder_.getEntryBuilder().CreateAlloca(argumentValue->getType());
						// TODO: take into account alignment?
						if (argInfo.getIndirectAlign() > allocaInst->getAlignment()) {
							allocaInst->setAlignment(argInfo.getIndirectAlign());
						}
						irCallArgs[firstIRArg] = allocaInst;
						
						builder_.getBuilder().CreateStore(argumentValue, allocaInst);
					} else {
						// We want to avoid creating an unnecessary temporary+copy here;
						// however, we need one in three cases:
						// 1. If the argument is not byval, and we are required to copy the
						//		source.	(This case doesn't occur on any common architecture.)
						// 2. If the argument is byval, RV is not sufficiently aligned, and
						//		we cannot force it to be sufficiently aligned.
						// 3. If the argument is byval, but RV is located in an address space
						//		different than that of the argument (0).
						llvm_unreachable("TODO");
					}
					break;
				}

				case ArgInfo::Ignore:
					assert(numIRArgs == 0);
					break;

				case ArgInfo::ExtendInteger:
				case ArgInfo::Direct: {
					const auto coerceType = argInfo.getCoerceToType();
					
					// Handle the trivial case.
					if (!coerceType.isStruct() &&
					    coerceType == argumentType &&
					    argInfo.getDirectOffset() == 0) {
						assert(numIRArgs == 1);
						auto value = argumentValue;
						
						const auto llvmArgType = typeInfo_.getLLVMType(argumentType);
						
						// We might have to widen integers, but we should never truncate.
						if (llvmArgType != value->getType() &&
						    value->getType()->isIntegerTy()) {
							value = builder_.getBuilder().CreateZExt(value,
							                                         llvmArgType);
						}
						
						// If the argument doesn't match, perform a bitcast to coerce it.
						// This can happen due to trivial type mismatches.
						if (llvmArgType != value->getType() &&
						    firstIRArg < functionIRMapping_.totalIRArgs()) {
							value = builder_.getBuilder().CreateBitCast(value,
							                                            llvmArgType);
						}
						
						irCallArgs[firstIRArg] = value;
						break;
					}
					
					llvm::Value* sourcePtr = nullptr;
					if (!isArgumentInMemory) {
						sourcePtr = builder_.getEntryBuilder().CreateAlloca(argumentValue->getType());
						// TODO: take into account alignment?
						builder_.getBuilder().CreateStore(argumentValue, sourcePtr);
					} else {
						sourcePtr = argumentValue;
					}
					
					// If the value is offset in memory, apply the offset now.
					if (argInfo.getDirectOffset() != 0) {
						sourcePtr = builder_.getBuilder().CreateBitCast(sourcePtr, llvm::PointerType::getUnqual(typeInfo_.getLLVMType(Int8Ty)));
						sourcePtr = builder_.getBuilder().CreateConstGEP1_32(sourcePtr, argInfo.getDirectOffset());
						sourcePtr = builder_.getBuilder().CreateBitCast(sourcePtr, llvm::PointerType::getUnqual(typeInfo_.getLLVMType(coerceType)));
					}
					
					// Fast-isel and the optimizer generally like scalar values better than
					// FCAs, so we flatten them if this is safe to do for this argument.
					if (coerceType.isStruct() && argInfo.isDirect() && argInfo.getCanBeFlattened()) {
						const auto sourceSize = typeInfo_.getTypeSize(argumentType);
						const auto destSize = typeInfo_.getTypeSize(coerceType);
						
						// If the source type is smaller than the destination type of the
						// coerce-to logic, copy the source value into a temp alloca the size
						// of the destination type to allow loading all of it. The bits past
						// the source value are left undef.
						if (sourceSize < destSize) {
							const auto tempAlloca = builder_.getEntryBuilder().CreateAlloca(typeInfo_.getLLVMType(coerceType));
							tempAlloca->setName(sourcePtr->getName() + ".coerce");
							builder_.getBuilder().CreateMemCpy(tempAlloca, sourcePtr, sourceSize, 0);
							sourcePtr = tempAlloca;
						} else {
							sourcePtr = builder_.getBuilder().CreateBitCast(sourcePtr, llvm::PointerType::getUnqual(typeInfo_.getLLVMType(coerceType)));
						}
						
						assert(numIRArgs == coerceType.structMembers().size());
						
						for (size_t i = 0; i < numIRArgs; i++) {
							const auto elementPtr = builder_.getBuilder().CreateConstGEP2_32(sourcePtr, 0, i);
							const auto loadInst = builder_.getBuilder().CreateLoad(elementPtr);
							// We don't know what we're loading from.
							loadInst->setAlignment(1);
							irCallArgs[firstIRArg + i] = loadInst;
						}
					} else {
						// In the simple case, just pass the coerced loaded value.
						assert(numIRArgs == 1);
						// FIXME: irCallArgs[firstIRArg] = createCoercedLoad(sourcePtr, coerceType);
						llvm_unreachable("TODO");
					}
					break;
				}

				case ArgInfo::Expand: {
// 					unsigned IRArgPos = FirstIRArg;
// 					ExpandTypeToArgs(I->Ty, RV, IRFuncTy, IRCallArgs, IRArgPos);
// 					assert(IRArgPos == FirstIRArg + NumIRArgs);
					llvm_unreachable("TODO");
					break;
				}
			}
		}
		
		return irCallArgs;
	}
	
	llvm::Value*
	Caller::decodeReturnValue(llvm::Value* const /*returnValue*/) {
		llvm_unreachable("TODO");
	}
	
}
