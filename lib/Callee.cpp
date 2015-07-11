
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Builder.hpp>
#include <llvm-abi/Callee.hpp>
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
		return allocaInst;
	}
	
	static
	llvm::AllocaInst* createMemTemp(const ABITypeInfo& typeInfo,
	                                Builder& builder,
	                                const Type type,
	                                const llvm::Twine& name = "") {
		const auto allocaInst = createTempAlloca(typeInfo,
		                                         builder,
		                                         type,
		                                         name);
		allocaInst->setAlignment(typeInfo.getTypeRequiredAlign(type).asBytes());
		return allocaInst;
	}
	
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
	
	void expandTypeFromArgs(const ABITypeInfo& typeInfo,
	                        Builder& builder,
	                        const Type type,
	                        llvm::Value* const alloca,
	                        llvm::SmallVectorImpl<llvm::Value* const>::iterator& iterator) {
		assert(type != VoidTy);
		
		if (type.isArray()) {
			for (size_t i = 0; i < type.arrayElementCount(); i++) {
				const auto elementAddress = builder.getBuilder().CreateConstGEP2_32(alloca, 0, i);
				const auto elementIRType = typeInfo.getLLVMType(type.arrayElementType());
				const auto castAddress = builder.getBuilder().CreateBitCast(elementAddress, elementIRType->getPointerTo());
				expandTypeFromArgs(typeInfo, builder, type,
				                   castAddress, iterator);
			}
		} else if (type.isStruct()) {
			assert(!type.hasFlexibleArrayMember() &&
			       "Cannot expand structure with flexible array.");
			
			for (size_t i = 0; i < type.structMembers().size(); i++) {
				const auto& field = type.structMembers()[i];
				// Skip zero length bitfields.
				if (field.isBitField() &&
				    field.bitFieldWidth().asBits() == 0) {
					continue;
				}
				assert(!field.isBitField() &&
 				       "Cannot expand structure with bit-field members.");
				const auto fieldAddress = builder.getBuilder().CreateConstGEP2_32(alloca, 0, i);
				expandTypeFromArgs(typeInfo, builder,
				                   field.type(), fieldAddress,
				                   iterator);
			}
		} else if (type.isUnion()) {
			// Unions can be here only in degenerative cases - all the fields are same
			// after flattening. Thus we have to use the "largest" field.
			auto largestSize = DataSize::Zero();
			Type largestType = VoidTy;
			
			for (const auto field: type.unionMembers()) {
				// Skip zero length bitfields.
// 				if (field.isBitField() &&
// 				    field.bitFieldWidth().asBits() == 0) {
// 					continue;
// 				}
// 				assert(!field.isBitField() &&
// 				       "Cannot expand structure with bit-field members.");
				const auto fieldSize = typeInfo.getTypeAllocSize(field);
				if (largestSize < fieldSize) {
					largestSize = fieldSize;
					largestType = field;
				}
			}
			
			if (largestType == VoidTy) {
				return;
			}
			
			const auto irType = typeInfo.getLLVMType(largestType);
			const auto castAddress = builder.getBuilder().CreateBitCast(alloca,
			                                                            irType->getPointerTo());
			expandTypeFromArgs(typeInfo, builder, largestType,
			                   castAddress, iterator);
		} else if (type.isComplex()) {
			llvm_unreachable("TODO");
		} else {
			const auto value = *iterator++;
			const auto storeInst = builder.getBuilder().CreateStore(value, alloca);
			storeInst->setAlignment(typeInfo.getTypeRequiredAlign(type).asBytes());
		}
	}
	
	Callee::Callee(const ABITypeInfo& typeInfo,
	               const FunctionType& functionType,
	               const FunctionIRMapping& functionIRMapping,
	               Builder& builder)
	: typeInfo_(typeInfo),
	functionType_(functionType),
	functionIRMapping_(functionIRMapping),
	builder_(builder) { }
	
	llvm::SmallVector<llvm::Value*, 8>
	Callee::decodeArguments(llvm::ArrayRef<llvm::Value*> encodedArguments) {
		assert(functionIRMapping_.totalIRArgs() == encodedArguments.size());
		
		// If we're using inalloca, all the memory arguments are GEPs off of the last
		// parameter, which is a pointer to the complete memory area.
		llvm::Value* argStruct = nullptr;
		if (functionIRMapping_.hasInallocaArg()) {
			argStruct = encodedArguments[functionIRMapping_.inallocaArgIndex()];
			//assert(argStruct->getType() == FI.getArgStruct()->getPointerTo());
		}
		
		// Name the struct return parameter.
		if (functionIRMapping_.hasStructRetArg()) {
			const auto structRetArgValue = encodedArguments[functionIRMapping_.structRetArgIndex()];
			structRetArgValue->setName("agg.result");
		}
		
		llvm::SmallVector<llvm::Value*, 8> arguments;
		
		for (size_t argIndex = 0; argIndex < functionType_.argumentTypes().size(); argIndex++) {
			const auto& argumentType = functionType_.argumentTypes()[argIndex];
			const auto& argInfo = functionIRMapping_.arguments()[argIndex].argInfo;
			
			unsigned firstIRArg, numIRArgs;
			std::tie(firstIRArg, numIRArgs) = functionIRMapping_.getIRArgRange(argIndex);
			
			switch (argInfo.getKind()) {
				case ArgInfo::InAlloca: {
					assert(numIRArgs == 0);
					const auto value = builder_.getBuilder().CreateStructGEP(argStruct,
					                                                         argInfo.getInAllocaFieldIndex(),
					                                                         "inalloca.arg");
					arguments.push_back(value);
					break;
				}
				case ArgInfo::Indirect: {
					assert(numIRArgs == 1);
					auto value = encodedArguments[firstIRArg];
					
					if (argumentType.isArray() || argumentType.isStruct()) {
						// Aggregates and complex variables are accessed by reference.
						// All we need to do is realign the value, if requested.
						if (argInfo.getIndirectRealign()) {
							const auto alignedTempAlloca = createMemTemp(typeInfo_,
							                                             builder_,
							                                             argumentType,
							                                             "coerce");
							
							// Copy from the incoming argument pointer to the temporary with the
							// appropriate alignment.
							//
							// FIXME: We should have a common utility for generating an aggregate
							// copy.
							const auto i8PtrType = builder_.getBuilder().getInt8PtrTy();
							
							const auto typeSize = typeInfo_.getTypeAllocSize(argumentType);
							const auto dest = builder_.getBuilder().CreateBitCast(alignedTempAlloca,
							                                                      i8PtrType);
							const auto source = builder_.getBuilder().CreateBitCast(value,
							                                                        i8PtrType);
							builder_.getBuilder().CreateMemCpy(dest, source,
							                                   llvm::ConstantInt::get(typeInfo_.getLLVMType(IntPtrTy),
							                                                          typeSize.asBytes()),
							                                   argInfo.getIndirectAlign(),
							                                   false);
							value = alignedTempAlloca;
						}
						
						const auto typeAlign = typeInfo_.getTypeRequiredAlign(argumentType);
						const auto loadInst = builder_.getBuilder().CreateLoad(value);
						const auto loadAlign = std::max<size_t>(typeAlign.asBytes(),
						                                        argInfo.getIndirectAlign());
						loadInst->setAlignment(loadAlign);
						arguments.push_back(loadInst);
					} else {
						// Load scalar value from indirect argument.
						// TODO: this needs to handle issues such
						// as truncation of bool values, efficiently
						// loading vectors etc.
						const auto loadInst = builder_.getBuilder().CreateLoad(value);
						loadInst->setAlignment(argInfo.getIndirectAlign());
						arguments.push_back(loadInst);
					}
					break;
				}
				
				case ArgInfo::ExtendInteger:
				case ArgInfo::Direct: {
					const auto coerceType = argInfo.getCoerceToType();
					
					// If we have the trivial case, handle it with no muss and fuss.
					if (!coerceType.isStruct() &&
					    coerceType == argumentType &&
					    argInfo.getDirectOffset() == 0) {
						assert(numIRArgs == 1);
						
						auto value = encodedArguments[firstIRArg];
						
						// Ensure the argument is the correct type.
						if (value->getType() != typeInfo_.getLLVMType(coerceType)) {
							value = builder_.getBuilder().CreateBitCast(value, typeInfo_.getLLVMType(coerceType));
						}
						
// 						if (isPromoted) {
// 							value = emitArgumentDemotion(value);
// 						}
						
						if (value->getType() != typeInfo_.getLLVMType(argumentType)) {
							value = builder_.getBuilder().CreateBitCast(value, typeInfo_.getLLVMType(argumentType));
						}
						
						arguments.push_back(value);
						break;
					}
					
					const auto alloca = createMemTemp(typeInfo_,
					                                  builder_,
					                                  argumentType,
					                                  "coerce.mem");
					
					// The alignment we need to use is the max of the requested alignment for
					// the argument plus the alignment required by our access code below.
					const auto alignmentToUse = std::max(typeInfo_.getTypeRequiredAlign(coerceType),
					                                     typeInfo_.getTypeRequiredAlign(argumentType));
					alloca->setAlignment(alignmentToUse.asBytes());
					
					llvm::Value* destPtr = alloca; // Pointer to store into.
					auto destType = argumentType;
					
					// If the value is offset in memory, apply the offset now.
					if (argInfo.getDirectOffset() != 0) {
						destPtr = builder_.getBuilder().CreateBitCast(destPtr, llvm::PointerType::getUnqual(typeInfo_.getLLVMType(Int8Ty)));
						destPtr = builder_.getBuilder().CreateConstGEP1_32(destPtr, argInfo.getDirectOffset());
						destPtr = builder_.getBuilder().CreateBitCast(destPtr, llvm::PointerType::getUnqual(typeInfo_.getLLVMType(coerceType)));
						destType = coerceType;
					}
					
					// Fast-isel and the optimizer generally like scalar values better than
					// FCAs, so we flatten them if this is safe to do for this argument.
					if (argInfo.isDirect() &&
					    argInfo.getCanBeFlattened() &&
					    coerceType.isStruct() &&
					    coerceType.structMembers().size() > 1) {
						assert(coerceType.structMembers().size() == numIRArgs);
						
						const auto sourceSize = typeInfo_.getTypeAllocSize(coerceType);
						const auto destSize = typeInfo_.getTypeAllocSize(argumentType);
						
						if (sourceSize <= destSize) {
							destPtr = builder_.getBuilder().CreateBitCast(destPtr,
							                                              llvm::PointerType::getUnqual(typeInfo_.getLLVMType(coerceType)));
							
							for (size_t i = 0; i < coerceType.structMembers().size(); i++) {
								const auto argValue = encodedArguments[firstIRArg + i];
								argValue->setName("coerce" + llvm::Twine(i));
								const auto elementPtr = builder_.getBuilder().CreateConstGEP2_32(destPtr, 0, i);
								builder_.getBuilder().CreateStore(argValue, elementPtr);
							}
						} else {
							const auto tempAlloca = createTempAlloca(typeInfo_,
							                                         builder_,
							                                         coerceType,
							                                         "coerce");
							tempAlloca->setAlignment(alignmentToUse.asBytes());
							
							for (size_t i = 0; i < coerceType.structMembers().size(); i++) {
								const auto argValue = encodedArguments[firstIRArg + i];
								argValue->setName("coerce" + llvm::Twine(i));
								const auto elementPtr = builder_.getBuilder().CreateConstGEP2_32(tempAlloca, 0, i);
								builder_.getBuilder().CreateStore(argValue, elementPtr);
							}
							
							builder_.getBuilder().CreateMemCpy(destPtr,
							                                   tempAlloca,
							                                   destSize.asBytes(),
							                                   alignmentToUse.asBytes());
						}
					} else {
						// Simple case, just do a coerced store of the argument into the alloca.
						assert(numIRArgs == 1);
						
						const auto argValue = encodedArguments[firstIRArg];
						argValue->setName("coerce");
						createCoercedStore(typeInfo_,
						                   builder_,
						                   argValue,
						                   destPtr,
						                   coerceType,
						                   destType);
					}
					
					arguments.push_back(builder_.getBuilder().CreateLoad(alloca));
					break;
				}
				case ArgInfo::Expand: {
					// If this structure was expanded into multiple arguments then
					// we need to create a temporary and reconstruct it from the
					// arguments.
					const auto alloca = createMemTemp(typeInfo_,
					                                  builder_,
					                                  argumentType,
					                                  "expand.dest.arg");
					
					auto iterator = encodedArguments.begin() + firstIRArg;
					expandTypeFromArgs(typeInfo_, builder_,
					                   argumentType,
					                   alloca,
					                   iterator);
					assert(iterator == encodedArguments.begin() + firstIRArg + numIRArgs);
					
					const auto loadInst = builder_.getBuilder().CreateLoad(alloca);
					loadInst->setAlignment(typeInfo_.getTypeRequiredAlign(argumentType).asBytes());
					arguments.push_back(loadInst);
					break;
				}
				case ArgInfo::Ignore: {
					assert(numIRArgs == 0);
					arguments.push_back(llvm::UndefValue::get(typeInfo_.getLLVMType(argumentType)));
					break;
				}
			}
		}
		
		return arguments;
	}
	
	llvm::Value*
	Callee::encodeReturnValue(llvm::Value* const returnValue,
	                          llvm::ArrayRef<llvm::Value*> encodedArguments,
	                          llvm::Value* const /*returnValuePtr*/) {
		assert(returnValue != nullptr);
		assert(functionIRMapping_.totalIRArgs() == encodedArguments.size());
		
		const auto returnType = functionType_.returnType();
		const auto& returnArgInfo = functionIRMapping_.returnArgInfo();
		
		switch (returnArgInfo.getKind()) {
			case ArgInfo::InAlloca: {
				assert(returnType.isArray() || returnType.isStruct());
				// Aggregrates get evaluated directly into the destination.
				// Sometimes we need to return the sret value in a register, though.
				if (returnArgInfo.getInAllocaSRet()) {
					const auto argStruct = encodedArguments.back();
					const auto structRet = builder_.getBuilder().CreateStructGEP(argStruct,
					                                                             returnArgInfo.getInAllocaFieldIndex());
					return builder_.getBuilder().CreateLoad(structRet, "sret");
				} else {
					return llvm::UndefValue::get(typeInfo_.getLLVMType(VoidTy));
				}
			}
			case ArgInfo::Indirect: {
				const size_t argIndex = returnArgInfo.isSRetAfterThis() ? 1 : 0;
				assert(argIndex < encodedArguments.size());
				const auto indirectArg = encodedArguments[argIndex];
				
				// Value is returned by storing it into the struct-ret pointer argument.
				assert(indirectArg->getType() == returnValue->getType()->getPointerTo());
				builder_.getBuilder().CreateStore(returnValue, indirectArg);
				
				// (Nothing is returned by-value.)
				return llvm::UndefValue::get(typeInfo_.getLLVMType(VoidTy));
			}
			case ArgInfo::ExtendInteger:
			case ArgInfo::Direct: {
				const auto coerceType = returnArgInfo.getCoerceToType();
				
				const auto returnLLVMType = typeInfo_.getLLVMType(returnType);
				const auto coerceLLVMType = typeInfo_.getLLVMType(coerceType);
				
				if (coerceLLVMType == returnLLVMType &&
				    returnArgInfo.getDirectOffset() == 0) {
					// Nothing to do.
					return returnValue;
				} else {
					// For more complex cases, store the value
					// into a temporary alloca and then perform
					// a coerced load from it.
					llvm::Value* sourcePtr = createMemTemp(typeInfo_,
					                                       builder_,
					                                       returnType,
					                                       "coerce");
					builder_.getBuilder().CreateStore(returnValue, sourcePtr);
					
					auto sourceType = returnType;
					
					if (returnArgInfo.getDirectOffset() != 0) {
						sourcePtr = builder_.getBuilder().CreateBitCast(sourcePtr, llvm::PointerType::getUnqual(typeInfo_.getLLVMType(Int8Ty)));
						sourcePtr = builder_.getBuilder().CreateConstGEP1_32(sourcePtr, returnArgInfo.getDirectOffset());
						sourcePtr = builder_.getBuilder().CreateBitCast(sourcePtr, llvm::PointerType::getUnqual(typeInfo_.getLLVMType(coerceType)));
						sourceType = coerceType;
					}
					
					return createCoercedLoad(typeInfo_,
					                         builder_,
					                         sourcePtr,
					                         sourceType,
					                         coerceType);
				}
			}
			case ArgInfo::Ignore:
				return llvm::UndefValue::get(typeInfo_.getLLVMType(functionType_.returnType()));
			case ArgInfo::Expand:
				llvm_unreachable("Invalid ABI kind for return argument");
		}
		
		llvm_unreachable("Unhandled ArgInfo::Kind");
	}
	
}
