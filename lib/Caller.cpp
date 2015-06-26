
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Builder.hpp>
#include <llvm-abi/Caller.hpp>
#include <llvm-abi/FunctionIRMapping.hpp>
#include <llvm-abi/FunctionType.hpp>

namespace llvm_abi {
	
	static
	llvm::AllocaInst* createTempAlloca(const ABITypeInfo& typeInfo,
	                                   Builder& builder,
	                                   const Type type,
	                                   const llvm::Twine& name = "") {
		const auto allocaInst = builder.getEntryBuilder().CreateAlloca(typeInfo.getLLVMType(type));
		allocaInst->setName(name);
		allocaInst->setAlignment(typeInfo.getTypeRequiredAlign(type).asBytes());
		return allocaInst;
	}
	
	Caller::Caller(const ABITypeInfo& typeInfo,
	               const FunctionType& functionType,
	               const FunctionIRMapping& functionIRMapping,
	               Builder& builder)
	: typeInfo_(typeInfo),
	functionType_(functionType),
	functionIRMapping_(functionIRMapping),
	builder_(builder) { }
	
	/// EnterStructPointerForCoercedAccess - Given a struct pointer that we are
	/// accessing some number of bytes out of it, try to gep into the struct to get
	/// at its inner goodness. Dive as deep as possible without entering an element
	/// with an in-memory size smaller than destSize.
	static std::pair<llvm::Value*, Type>
	enterStructPointerForCoercedAccess(const ABITypeInfo& typeInfo,
	                                   Builder& builder,
	                                   llvm::Value* const sourcePtr,
	                                   const Type sourceStructType,
	                                   const DataSize destSize) {
		if (sourceStructType.structMembers().empty()) {
			// We can't dive into a zero-element struct.
			return std::make_pair(sourcePtr, sourceStructType);
		}
		
		const auto firstElementType = sourceStructType.structMembers()[0].type();
		
		// If the first elt is at least as large as what we're looking for, or if the
		// first element is the same size as the whole struct, we can enter it. The
		// comparison must be made on the store size and not the alloca size. Using
		// the alloca size may overstate the size of the load.
		const auto firstElementSize = typeInfo.getTypeStoreSize(firstElementType);
		if (firstElementSize < destSize &&
		    firstElementSize < typeInfo.getTypeStoreSize(sourceStructType)) {
			return std::make_pair(sourcePtr, sourceStructType);
		}
		
		// GEP into the first element.
		const auto diveSourcePtr = builder.getBuilder().CreateConstGEP2_32(sourcePtr, 0, 0, "coerce.dive");
		
		// If the first element is a struct, recurse.
		if (firstElementType.isStruct()) {
			return enterStructPointerForCoercedAccess(typeInfo,
			                                          builder,
			                                          diveSourcePtr,
			                                          firstElementType,
			                                          destSize);
		} else {
			return std::make_pair(diveSourcePtr, firstElementType);
		}
	}
	
	/// CoerceIntOrPtrToIntOrPtr - Convert a value Val to the specific Ty where both
	/// are either integers or pointers. This does a truncation of the value if it
	/// is too large or a zero extension if it is too small.
	///
	/// This behaves as if the value were coerced through memory, so on big-endian
	/// targets the high bits are preserved in a truncation, while little-endian
	/// targets preserve the low bits.
	static llvm::Value*
	coerceIntOrPtrToIntOrPtr(const ABITypeInfo& typeInfo,
	                         Builder& builder,
	                         llvm::Value* value,
	                         const Type sourceType,
	                         const Type destType) {
		if (value->getType() == typeInfo.getLLVMType(destType)) {
			return value;
		}
		
		if (sourceType.isPointer()) {
			// If this is Pointer->Pointer avoid conversion to and from int.
			if (destType.isPointer()) {
				return builder.getBuilder().CreateBitCast(value,
				                                          typeInfo.getLLVMType(destType),
				                                          "coerce.val");
			}
			
			// Convert the pointer to an integer so we can play with its width.
			value = builder.getBuilder().CreatePtrToInt(value,
			                                            typeInfo.getLLVMType(IntPtrTy),
			                                            "coerce.val.pi");
		}
		
		const auto destIntLLVMType = typeInfo.getLLVMType(destType.isPointer() ? IntPtrTy : destType);
		
		if (value->getType() != destIntLLVMType) {
			if (typeInfo.isBigEndian()) {
				// Preserve the high bits on big-endian targets.
				// That is what memory coercion does.
				const auto sourceSize = typeInfo.getTypeRawSize(sourceType);
				const auto destSize = typeInfo.getTypeRawSize(destType);
				
				if (sourceSize > destSize) {
					value = builder.getBuilder().CreateLShr(value,
					                                        (sourceSize - destSize).asBits(),
					                                        "coerce.highbits");
					value = builder.getBuilder().CreateTrunc(value,
					                                         destIntLLVMType,
					                                         "coerce.val.ii");
				} else {
					value = builder.getBuilder().CreateZExt(value,
					                                        destIntLLVMType,
					                                        "coerce.val.ii");
					value = builder.getBuilder().CreateShl(value,
					                                       (destSize - sourceSize).asBits(),
					                                       "coerce.highbits");
				}
			} else {
				// Little-endian targets preserve the low bits. No shifts required.
				value = builder.getBuilder().CreateIntCast(value,
				                                           destIntLLVMType,
				                                           false,
				                                           "coerce.val.ii");
			}
		}
		
		if (destType.isPointer()) {
			value = builder.getBuilder().CreateIntToPtr(value,
			                                            typeInfo.getLLVMType(destType),
			                                            "coerce.val.ip");
		}
		
		return value;
	}
	
	/// createCoercedLoad - Create a load from \arg sourcePtr interpreted as
	/// a pointer to an object of type \arg destType.
	///
	/// This safely handles the case when the src type is smaller than the
	/// destination type; in this situation the values of bits which not
	/// present in the src are undefined.
	static llvm::Value* createCoercedLoad(const ABITypeInfo& typeInfo,
	                                      Builder& builder,
	                                      llvm::Value* sourcePtr,
	                                      Type sourceType,
	                                      const Type destType) {
		// If source and destination types are the same, just do a load.
		if (typeInfo.getLLVMType(sourceType) == typeInfo.getLLVMType(destType)) {
			return builder.getBuilder().CreateLoad(sourcePtr);
		}
		
		const auto destSize = typeInfo.getTypeAllocSize(destType);
		
		if (sourceType.isStruct()) {
			const auto result = enterStructPointerForCoercedAccess(typeInfo,
			                                                       builder,
			                                                       sourcePtr,
			                                                       sourceType,
			                                                       destSize);
			sourcePtr = result.first;
			sourceType = result.second;
		}
		
		const auto sourceSize = typeInfo.getTypeAllocSize(sourceType);
		
		// If the source and destination are integer or pointer types, just do an
		// extension or truncation to the desired type.
		if ((destType.isInteger() || destType.isPointer()) &&
		    (sourceType.isInteger() || sourceType.isPointer())) {
			const auto loadInst = builder.getBuilder().CreateLoad(sourcePtr);
			return coerceIntOrPtrToIntOrPtr(typeInfo,
			                                builder,
			                                loadInst,
			                                sourceType,
			                                destType);
		}
		
		// If load is legal, just bitcast the src pointer.
		if (sourceSize >= destSize) {
			// Generally sourceSize is never greater than destSize, since this means we are
			// losing bits. However, this can happen in cases where the structure has
			// additional padding, for example due to a user specified alignment.
			//
			// FIXME: Assert that we aren't truncating non-padding bits when have access
			// to that information.
			const auto casted = builder.getBuilder().CreateBitCast(sourcePtr,
			                                                       llvm::PointerType::getUnqual(typeInfo.getLLVMType(destType)));
			const auto loadInst = builder.getBuilder().CreateLoad(casted);
			// FIXME: Use better alignment / avoid requiring aligned load.
			loadInst->setAlignment(1);
			return loadInst;
		} else {
			// Otherwise do coercion through memory. This is stupid, but
			// simple.
			const auto tmpAlloca = createTempAlloca(typeInfo,
			                                        builder,
			                                        destType,
			                                        "coerce.mem.load");
			const auto i8PtrType = typeInfo.getLLVMType(Int8Ty)->getPointerTo();
			const auto casted = builder.getBuilder().CreateBitCast(tmpAlloca, i8PtrType);
			const auto sourceCasted = builder.getBuilder().CreateBitCast(sourcePtr, i8PtrType);
			// FIXME: Use better alignment.
			builder.getBuilder().CreateMemCpy(casted, sourceCasted,
			                                  llvm::ConstantInt::get(typeInfo.getLLVMType(IntPtrTy),
			                                                         sourceSize.asBytes()),
			                                  1, false);
			return builder.getBuilder().CreateLoad(tmpAlloca);
		}
	}
	
	// Function to store a first-class aggregate into memory. We prefer to
	// store the elements rather than the aggregate to be more friendly to
	// fast-isel.
	// FIXME: Do we need to recurse here?
	static void buildAggStore(Builder& builder,
	                          llvm::Value* const source,
	                          llvm::Value* const destPtr,
	                          const bool lowAlignment) {
		// Prefer scalar stores to first-class aggregate stores.
		if (const auto structType = llvm::dyn_cast<llvm::StructType>(source->getType())) {
			for (unsigned i = 0, e = structType->getNumElements(); i != e; ++i) {
				const auto elementPtr = builder.getBuilder().CreateConstGEP2_32(destPtr, 0, i);
				const auto element = builder.getBuilder().CreateExtractValue(source, i);
				const auto storeInst = builder.getBuilder().CreateStore(element,
				                                                        elementPtr);
				if (lowAlignment) {
					storeInst->setAlignment(1);
				}
			}
		} else {
			const auto storeInst = builder.getBuilder().CreateStore(source,
			                                                        destPtr);
			if (lowAlignment) {
				storeInst->setAlignment(1);
			}
		}
	}
	
	/// CreateCoercedStore - Create a store to \arg destPtr from \arg source,
	/// where the source and destination may have different types.
	///
	/// This safely handles the case when the src type is larger than the
	/// destination type; the upper bits of the src will be lost.
	static void createCoercedStore(const ABITypeInfo& typeInfo,
	                               Builder& builder,
	                               llvm::Value* const source,
	                               llvm::Value* destPtr,
	                               const Type sourceType,
	                               Type destType) {
		if (typeInfo.getLLVMType(sourceType) == typeInfo.getLLVMType(destType)) {
			builder.getBuilder().CreateStore(source, destPtr);
			return;
		}
		
		const auto sourceSize = typeInfo.getTypeAllocSize(sourceType);
		
		if (destType.isStruct()) {
			const auto result = enterStructPointerForCoercedAccess(typeInfo,
			                                                       builder,
			                                                       destPtr,
			                                                       destType,
			                                                       sourceSize);
			destPtr = result.first;
			destType = result.second;
		}
		
		// If the source and destination are integer or pointer types, just do an
		// extension or truncation to the desired type.
		if ((sourceType.isInteger() || sourceType.isPointer()) &&
		    (destType.isInteger() || destType.isPointer())) {
			const auto coercedSource = coerceIntOrPtrToIntOrPtr(typeInfo,
			                                                    builder,
			                                                    source,
			                                                    sourceType,
			                                                    destType);
			builder.getBuilder().CreateStore(coercedSource, destPtr);
			return;
		}
		
		const auto destSize = typeInfo.getTypeAllocSize(destType);
		
		// If store is legal, just bitcast the src pointer.
		if (sourceSize <= destSize) {
			const auto sourcePtrType =
				llvm::PointerType::getUnqual(typeInfo.getLLVMType(sourceType));
			const auto castedDestPtr =
				builder.getBuilder().CreateBitCast(destPtr,
				                                   sourcePtrType);
			buildAggStore(builder,
			              source,
			              castedDestPtr,
			              /*lowAlignment=*/true);
		} else {
			// Otherwise do coercion through memory. This is stupid, but
			// simple.

			// Generally SrcSize is never greater than DstSize, since this means we are
			// losing bits. However, this can happen in cases where the structure has
			// additional padding, for example due to a user specified alignment.
			//
			// FIXME: Assert that we aren't truncating non-padding bits when have access
			// to that information.
			const auto tempAlloca = createTempAlloca(typeInfo,
			                                         builder,
			                                         sourceType,
			                                         "coerce.mem.store");
			builder.getBuilder().CreateStore(source, tempAlloca);
			const auto i8PtrType = builder.getBuilder().getInt8PtrTy();
			const auto casted = builder.getBuilder().CreateBitCast(tempAlloca, i8PtrType);
			const auto destCasted = builder.getBuilder().CreateBitCast(destPtr, i8PtrType);
			// FIXME: Use better alignment.
			builder.getBuilder().CreateMemCpy(destCasted,
			                                  casted,
			                                  llvm::ConstantInt::get(typeInfo.getLLVMType(IntPtrTy),
			                                                         destSize.asBytes()),
			                                  1, false);
		}
	}
	
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
			
			assert(argumentValue->getType() == typeInfo_.getLLVMType(argumentType));
			
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
						const auto allocaInst = createTempAlloca(typeInfo_,
						                                         builder_,
						                                         argumentType,
						                                         "indirect.arg.mem");
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
						sourcePtr = createTempAlloca(typeInfo_,
						                             builder_,
						                             argumentType,
						                             "coerce.arg.source");
						builder_.getBuilder().CreateStore(argumentValue, sourcePtr);
					} else {
						assert(argumentValue->getType() == typeInfo_.getLLVMType(argumentType)->getPointerTo());
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
						const auto sourceSize = typeInfo_.getTypeAllocSize(argumentType);
						const auto destSize = typeInfo_.getTypeAllocSize(coerceType);
						
						// If the source type is smaller than the destination type of the
						// coerce-to logic, copy the source value into a temp alloca the size
						// of the destination type to allow loading all of it. The bits past
						// the source value are left undef.
						if (sourceSize < destSize) {
							const auto tempAlloca = createTempAlloca(typeInfo_,
							                                         builder_,
							                                         coerceType);
							tempAlloca->setName(sourcePtr->getName() + ".coerce");
							builder_.getBuilder().CreateMemCpy(tempAlloca,
							                                   sourcePtr,
							                                   sourceSize.asBytes(),
							                                   0);
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
						irCallArgs[firstIRArg] = createCoercedLoad(typeInfo_,
						                                           builder_,
						                                           sourcePtr,
						                                           argumentType,
						                                           coerceType);
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
	Caller::decodeReturnValue(llvm::ArrayRef<llvm::Value*> encodedArguments,
	                          llvm::Value* const encodedReturnValue,
	                          llvm::Value* const returnValuePtr) {
		const auto& returnArgInfo = functionIRMapping_.returnArgInfo();
		const auto returnType = functionType_.returnType();
		switch (returnArgInfo.getKind()) {
			case ArgInfo::InAlloca: {
				llvm_unreachable("TODO");
			}
			case ArgInfo::Indirect: {
				const auto returnValuePointer = encodedArguments[functionIRMapping_.structRetArgIndex()];
				return builder_.getBuilder().CreateLoad(returnValuePointer);
			}
			case ArgInfo::Ignore: {
				assert(encodedReturnValue->getType()->isVoidTy());
				return encodedReturnValue;
			}
			case ArgInfo::ExtendInteger:
			case ArgInfo::Direct: {
				const auto coerceType = returnArgInfo.getCoerceToType();
				if (coerceType == returnType &&
				    returnArgInfo.getDirectOffset() == 0) {
					if (returnType.isArray() || returnType.isStruct()) {
						auto destPtr = returnValuePtr;
						
						if (destPtr == nullptr) {
							destPtr = createTempAlloca(typeInfo_,
							                           builder_,
							                           returnType,
							                           "agg.tmp");
						}
						
						buildAggStore(builder_,
						              encodedReturnValue,
						              destPtr,
						              /*lowAlignment=*/false);
						
						return builder_.getBuilder().CreateLoad(destPtr);
					} else {
						// If the argument doesn't match, perform a bitcast to coerce it.  This
						// can happen due to trivial type mismatches.
						auto castReturnValue = encodedReturnValue;
						if (castReturnValue->getType() != typeInfo_.getLLVMType(returnType)) {
							castReturnValue = builder_.getBuilder().CreateBitCast(castReturnValue,
							                                                      typeInfo_.getLLVMType(returnType));
						}
						return castReturnValue;
					}
				}
				
				auto destPtr = returnValuePtr;
				
				if (destPtr == nullptr) {
					destPtr = createTempAlloca(typeInfo_,
					                           builder_,
					                           returnType,
					                           "coerce");
				}
				
				// If the value is offset in memory, apply the offset now.
				llvm::Value* storePtr = destPtr;
				if (returnArgInfo.getDirectOffset() != 0) {
					storePtr = builder_.getBuilder().CreateBitCast(storePtr, builder_.getBuilder().getInt8PtrTy());
					storePtr = builder_.getBuilder().CreateConstGEP1_32(storePtr, returnArgInfo.getDirectOffset());
					storePtr = builder_.getBuilder().CreateBitCast(storePtr, llvm::PointerType::getUnqual(typeInfo_.getLLVMType(coerceType)));
				}
				
				createCoercedStore(typeInfo_,
				                   builder_,
				                   encodedReturnValue,
				                   storePtr,
				                   coerceType,
				                   returnType);
				
				return builder_.getBuilder().CreateLoad(destPtr);
			}

			case ArgInfo::Expand:
				llvm_unreachable("Invalid ABI kind for return argument");
		}

		llvm_unreachable("Unhandled ArgInfo::Kind");
	}
	
}
