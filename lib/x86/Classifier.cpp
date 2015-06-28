#include <algorithm>
#include <cstddef>

#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/ErrorHandling.h>

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/ArgInfo.hpp>
#include <llvm-abi/DataSize.hpp>
#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <llvm-abi/x86/ArgClass.hpp>
#include <llvm-abi/x86/Classification.hpp>
#include <llvm-abi/x86/Classifier.hpp>

namespace llvm_abi {
	
	namespace x86_64 {
		
		bool isIntegralType(const Type type) {
			return type.isPointer() ||
			       type.isInteger() ||
			       type.isFloatingPoint() ||
			       type.isVector();
		}
		
		bool isAggregateTypeForABI(const Type type) {
			return !isIntegralType(type);
		}
		
		bool isPromotableIntegerType(const Type type) {
			return type == BoolTy ||
			       type == CharTy ||
			       type == SCharTy ||
			       type == UCharTy ||
			       type == ShortTy ||
			       type == UShortTy;
		}
		
		/**
		 * \brief Get field containing the offset given.
		 */
		size_t getFieldContainingOffset(llvm::ArrayRef<DataSize> fieldOffsets,
		                                const DataSize offset) {
			// Get field containing the offset given.
			size_t fieldIndex = 0;
			
			while ((fieldIndex + 1) < fieldOffsets.size()) {
				if (fieldOffsets[fieldIndex] <= offset &&
				    fieldOffsets[fieldIndex + 1] > offset) {
					break;
				}
				fieldIndex++;
			}
			
			return fieldIndex;
		}
		
		static bool isEmptyRecord(Type type, bool allowArrays);
		
		/**
		 * \brief Check if a field is "empty".
		 * 
		 * Checks if the field is an unnamed bit-field or an array
		 * of empty record(s).
		 */
		static bool isEmptyField(const StructMember& structMember,
		                         const bool allowArrays) {
// 			if (structMember.isUnnamedBitfield()) {
// 				return true;
// 			}
			
			Type fieldType = structMember.type();
			
			// Constant arrays of empty records count as empty, strip them off.
			// Constant arrays of zero length always count as empty.
			if (allowArrays) {
				while (fieldType.isArray()) {
					if (fieldType.arrayElementCount() != 1) {
						break;
					}
					fieldType = fieldType.arrayElementType();
				}
			}
			
			if (!fieldType.isStruct()) {
				return false;
			}
			
			return isEmptyRecord(fieldType, allowArrays);
		}

		/**
		 * \brief Check if a struct contains only empty fields.
		 * 
		 * Note that a struct with a flexible array member is not
		 * considered empty.
		 */
		static bool isEmptyRecord(const Type type,
		                          const bool allowArrays) {
			if (!type.isStruct()) {
				return false;
			}
			
// 			if (type.isStructVariableSized()) {
// 				return false;
// 			}
			
			for (const auto& field: type.structMembers()) {
				if (!isEmptyField(field, allowArrays)) {
					return false;
				}
			}
			
			return true;
		}
		
		/// isSingleElementStruct - Determine if a structure is a "single
		/// element struct", i.e. it has exactly one non-empty field or
		/// exactly one field which is itself a single element
		/// struct. Structures with flexible array members are never
		/// considered single element structs.
		///
		/// \return The field declaration for the single non-empty field, if
		/// it exists.
		static Type getStructSingleElement(const ABITypeInfo& typeInfo,
		                                   const Type type) {
			if (!type.isStruct()) {
				return VoidTy;
			}
			
// 			if (type.isStructVariableSized()) {
// 				return VoidTy;
// 			}
			
			Type foundType = VoidTy;
			
			// Check for single element.
			for (const auto& field: type.structMembers()) {
				// Ignore empty fields.
				const bool allowArrays = true;
				if (isEmptyField(field, allowArrays)) {
					continue;
				}
				
				// If we already found an element then this
				// isn't a single-element struct.
				if (!foundType.isVoid()) {
					return VoidTy;
				}
				
				Type fieldType = field.type();
				
				// Treat single element arrays as the element.
				while (fieldType.isArray()) {
					if (fieldType.arrayElementCount() != 1) {
						break;
					}
					fieldType = fieldType.arrayElementType();
				}
				
				if (!isAggregateTypeForABI(fieldType)) {
					foundType = fieldType;
				} else {
					foundType = getStructSingleElement(typeInfo,
					                                   fieldType);
					if (foundType.isVoid()) {
						return VoidTy;
					}
				}
			}
			
			// We don't consider a struct a single-element struct
			// if it has padding beyond the element type.
			if (!foundType.isVoid() &&
			    typeInfo.getTypeAllocSize(foundType) != typeInfo.getTypeAllocSize(type)) {
				return VoidTy;
			}
			
			return foundType;
		}
		
		ArgInfo getIndirectReturnResult(const Type type) {
			// If this is a scalar LLVM value then assume LLVM will
			// pass it in the right place naturally.
			if (!isAggregateTypeForABI(type)) {
				return isPromotableIntegerType(type) ?
				       ArgInfo::getExtend(type) : ArgInfo::getDirect(type);
			}
			
			return ArgInfo::getIndirect(0);
		}
		
		ArgInfo getIndirectResult(const ABITypeInfo& typeInfo,
		                          const Type type,
		                          const unsigned freeIntRegs) {
			// If this is a scalar LLVM value then assume LLVM will pass it in the right
			// place naturally.
			//
			// This assumption is optimistic, as there could be free registers available
			// when we need to pass this argument in memory, and LLVM could try to pass
			// the argument in the free register. This does not seem to happen currently,
			// but this code would be much safer if we could mark the argument with
			// 'onstack'. See PR12193.
			if (!isAggregateTypeForABI(type) &&
			    (!type.isVector() || typeInfo.isLegalVectorType(type))) {
				return isPromotableIntegerType(type) ?
					ArgInfo::getExtend(type) : ArgInfo::getDirect(type);
			}
			
			// Compute the byval alignment. We specify the alignment of the byval in all
			// cases so that the mid-level optimizer knows the alignment of the byval.
			const auto align = std::max<DataSize>(typeInfo.getTypeRequiredAlign(type), DataSize::Bytes(8));
			
			// Attempt to avoid passing indirect results using byval when possible. This
			// is important for good codegen.
			//
			// We do this by coercing the value into a scalar type which the backend can
			// handle naturally (i.e., without using byval).
			//
			// For simplicity, we currently only do this when we have exhausted all of the
			// free integer registers. Doing this when there are free integer registers
			// would require more care, as we would have to ensure that the coerced value
			// did not claim the unused register. That would require either reording the
			// arguments to the function (so that any subsequent inreg values came first),
			// or only doing this optimization when there were no following arguments that
			// might be inreg.
			//
			// We currently expect it to be rare (particularly in well written code) for
			// arguments to be passed on the stack when there are still free integer
			// registers available (this would typically imply large structs being passed
			// by value), so this seems like a fair tradeoff for now.
			//
			// We can revisit this if the backend grows support for 'onstack' parameter
			// attributes. See PR12193.
			if (freeIntRegs == 0) {
				const auto size = typeInfo.getTypeAllocSize(type);
				
				// If this type fits in an eightbyte, coerce it into the matching integral
				// type, which will end up on the stack (with alignment 8).
				if (align.asBytes() == 8 && size.asBytes() <= 8) {
					return ArgInfo::getDirect(Type::FixedWidthInteger(size,
					                                                  /*isSigned=*/false));
				}
			}
			
			return ArgInfo::getIndirect(align.asBytes());
		}
		
		/// The ABI specifies that a value should be passed in a full vector XMM/YMM
		/// register. Pick an LLVM IR type that will be passed as a vector register.
		Type getByteVectorType(const ABITypeInfo& typeInfo, Type type) {
			// Wrapper structs/arrays that only contain vectors are passed just like
			// vectors; strip them off if present.
			const auto singleElementType = getStructSingleElement(typeInfo, type);
			if (!singleElementType.isVoid()) {
				type = singleElementType;
			}
			
			// If the preferred type is a 16-byte vector, prefer
			// to pass it.
			if (type.isVector()) {
				const auto width = typeInfo.getTypeRawSize(type);
				const auto elementType = type.vectorElementType();
				const auto elementSize = typeInfo.getTypeRawSize(elementType);
				if ((width.asBits() >= 128 && width.asBits() <= 256) &&
					(elementType.isFloat() || elementType.isDouble() ||
					 (elementType.isInteger() &&
					  (elementSize.asBits() == 8 ||
					   elementSize.asBits() == 16 ||
					   elementSize.asBits() == 32 ||
					   elementSize.asBits() == 64 ||
					   elementSize.asBits() == 128)))) {
					return type;
				}
			}
			
			return typeInfo.typeBuilder().getVectorTy(2, DoubleTy);
		}
		
		/// BitsContainNoUserData - Return true if the specified [start,end) bit range
		/// is known to either be off the end of the specified type or being in
		/// alignment padding.	The user type specified is known to be at most 128 bits
		/// in size, and have passed through X86_64ABIInfo::classify with a successful
		/// classification that put one of the two halves in the INTEGER class.
		///
		/// It is conservatively correct to return false.
		static bool bitsContainNoUserData(const ABITypeInfo& typeInfo,
		                                  const Type type,
		                                  const size_t startBit,
		                                  const size_t endBit) {
			assert(startBit <= endBit);
			
			// If the bytes being queried are off the end of the type, there is no user
			// data hiding here. This handles analysis of builtins, vectors and other
			// types that don't contain interesting padding.
			if (typeInfo.getTypeAllocSize(type).asBits() <= startBit) {
				return true;
			}
			
			if (type.isArray()) {
				const auto elementSize = typeInfo.getTypeAllocSize(type.arrayElementType());
				const auto elementCount = type.arrayElementCount();
				
				for (size_t i = 0; i < elementCount; i++) {
					const auto elementOffset = elementSize * i;
					if (elementOffset.asBits() >= endBit) {
						// If the element is after the span we care about, then we're done.
						break;
					}
					
					const size_t elementStart = (elementOffset.asBits() < startBit) ? (startBit - elementOffset.asBits()) : 0;
					
					if (!bitsContainNoUserData(typeInfo,
					                           type.arrayElementType(),
					                           elementStart,
					                           endBit - elementOffset.asBits())) {
						return false;
					}
				}
				
				// If it overlaps no elements, then it is safe to
				// process as padding.
				return true;
			}
			
			if (type.isStruct()) {
				const auto structOffsets = typeInfo.calculateStructOffsets(type.structMembers());
				
				// Verify that no field has data that overlaps the region of interest. Yes
				// this could be sped up a lot by being smarter about queried fields,
				// however we're only looking at structs up to 16 bytes, so we don't care
				// much.
				for (size_t i = 0; i < type.structMembers().size(); i++) {
					const auto& structMember = type.structMembers()[i];
					const auto fieldOffset = structOffsets[i];
					
					if (fieldOffset.asBits() >= endBit) {
						// If we found a field after the region we care about, then we're done.
						break;
					}
					
					const auto fieldStart = (fieldOffset.asBits() < startBit) ? (startBit - fieldOffset.asBits()) : 0;
					if (!bitsContainNoUserData(typeInfo,
					                           structMember.type(),
					                           fieldStart,
					                           endBit - fieldOffset.asBits())) {
						return false;
					}
				}
				
				// If nothing in this record overlapped the area
				// of interest, then we're clean.
				return true;
			}
			
			return false;
		}
		
		/**
		 * Return true if the specified LLVM IR type has a float member
		 * at the specified offset.  For example, {int,{float}} has a
		 * float at offset 4.  It is conservatively correct for this
		 * routine to return false.
		 */
		static bool containsFloatAtOffset(const ABITypeInfo& typeInfo,
		                                  const Type type,
		                                  const DataSize offset) {
			// Base case if we find a float.
			if (offset.asBytes() == 0 && type.isFloat()) {
				return true;
			}
			
			// If this is a struct, recurse into the field at the specified offset.
			if (type.isStruct()) {
				const auto fieldOffsets = typeInfo.calculateStructOffsets(type.structMembers());
				const auto fieldIndex = getFieldContainingOffset(fieldOffsets, offset);
				assert(fieldOffsets[fieldIndex] <= offset);
				const auto relativeOffset = offset - fieldOffsets[fieldIndex];
				return containsFloatAtOffset(typeInfo,
				                             type.structMembers()[fieldIndex].type(),
				                             relativeOffset);
			}
			
			// If this is an array, recurse into the field at the specified offset.
			if (type.isArray()) {
				const auto elementType = type.arrayElementType();
				const auto elementSize = typeInfo.getTypeAllocSize(elementType);
				const auto elementOffset = elementSize * (offset / elementSize);
				assert(elementOffset <= offset);
				const auto relativeOffset = offset - elementOffset;
				return containsFloatAtOffset(typeInfo,
				                             elementType,
				                             relativeOffset);
			}
			
			return false;
		}
		
		/**
		 * \brief Get Sse type to be passed in XMM register.
		 * 
		 * Return a type that will be passed by the backend in the low
		 * 8 bytes of an XMM register, corresponding to the Sse class.
		 */
		Type
		getSseTypeAtOffset(const ABITypeInfo& typeInfo,
		                   const Type type,
		                   const DataSize offset,
		                   const Type sourceType,
		                   const DataSize sourceOffset) {
			assert(sourceOffset.asBytes() == 0 ||
			       sourceOffset.asBytes() == 8);
			
			// The only three choices we have are either double,
			// <2 x float>, or float.
			
			if (bitsContainNoUserData(typeInfo,
			                          sourceType,
			                          sourceOffset.asBits() + 32,
			                          sourceOffset.asBits() + 64)) {
				// We pass as float if the last 4 bytes is just
				// padding.  This happens for structs that
				// contain 3 floats.
				return FloatTy;
			}
			
			// We want to pass as <2 x float> if the LLVM IR type
			// contains a float at offset+0 and offset+4.  Walk
			// the type to find out if this is the case.
			if (containsFloatAtOffset(typeInfo, type, offset) &&
			    containsFloatAtOffset(typeInfo, type, offset + DataSize::Bytes(4))) {
				return typeInfo.typeBuilder().getVectorTy(2, FloatTy);
			}
			
			return DoubleTy;
		}
		
		/**
		 * \brief Get integer type to be passed in an 8-byte GPR.
		 * 
		 * The ABI specifies that a value should be passed in an 8-byte
		 * general purpose register.  This means that we either have a
		 * scalar or we are talking about the high or low part of an
		 * up-to-16-byte struct.  This routine picks the best LLVM IR
		 * type to represent this, which may be i64 or may be anything
		 * else that the backend will pass in a GPR that works better
		 * (e.g. i8, %foo*, etc).
		 * 
		 * irType is an LLVM IR type that corresponds to (part of) the
		 * IR type for the source type. irOffset is an offset in bytes
		 * into the LLVM IR type that the 8-byte value references.
		 * irType may be null.
		 * 
		 * sourceType is the source-level type for the entire argument.
		 * sourceOffset is an offset into this that we're processing
		 * (which is always either 0 or 8).
		 */
		Type
		getINTEGERTypeAtOffset(const ABITypeInfo& typeInfo,
		                       const Type type,
		                       const DataSize offset,
		                       const Type sourceType,
		                       const DataSize sourceOffset) {
			assert(sourceOffset.asBytes() == 0 ||
			       sourceOffset.asBytes() == 8);
			
			// If we're dealing with an un-offset type, then
			// it means that we're returning an 8-byte unit
			// starting with it.  See if we can safely use it.
			if (offset.asBytes() == 0) {
				const auto typeSize = typeInfo.getTypeAllocSize(type);
				
				// Pointers and int64's always fill the 8-byte unit.
				if ((type.isPointer() || type.isInteger()) &&
				    typeSize.asBytes() == 8) {
					return type;
				}
				
				// If we have a 1/2/4-byte integer, we can use it
				// only if the rest of the goodness in the source
				// type is just tail padding.  This is allowed to
				// kick in for struct {double,int} on the int, but
				// not on struct{double,int,int} because we
				// wouldn't return the second int.
				if ((type.isPointer() || type.isInteger()) &&
				    (typeSize.asBytes() == 1 ||
				     typeSize.asBytes() == 2 || 
				     typeSize.asBytes() == 4)) {
					if (bitsContainNoUserData(typeInfo,
					                          sourceType,
					                          sourceOffset.asBits() + typeSize.asBits(),
					                          sourceOffset.asBits() + 64)) {
						return type;
					}
				}
			}
			
			if (type.isStruct() && offset < typeInfo.getTypeAllocSize(type)) {
				// If this is a struct, recurse into the field at the specified offset.
				const auto fieldOffsets = typeInfo.calculateStructOffsets(type.structMembers());
				const auto fieldIndex = getFieldContainingOffset(fieldOffsets, offset);
				assert(fieldOffsets[fieldIndex] <= offset);
				const auto relativeOffset = offset - fieldOffsets[fieldIndex];
				
				return getINTEGERTypeAtOffset(typeInfo,
				                              type.structMembers()[fieldIndex].type(),
				                              relativeOffset,
				                              sourceType,
				                              sourceOffset);
			}
			
			if (type.isArray()) {
				const auto elementType = type.arrayElementType();
				const auto elementSize = typeInfo.getTypeAllocSize(elementType);
				const auto elementOffset = elementSize * (offset / elementSize);
				assert(elementOffset <= offset);
				return getINTEGERTypeAtOffset(typeInfo,
				                              elementType,
				                              offset - elementOffset,
				                              sourceType,
				                              sourceOffset);
			}
			
			// Okay, we don't have any better idea of what to pass, so we pass this in an
			// integer register that isn't too big to fit the rest of the struct.
			const auto typeSize = typeInfo.getTypeAllocSize(sourceType);
			assert(typeSize != sourceOffset && "Empty field?");
			
			// It is always safe to classify this as an integer
			// type up to i64 that isn't larger than the structure.
			const auto intSize = std::min<DataSize>(typeSize - sourceOffset, DataSize::Bytes(8));
			return Type::FixedWidthInteger(intSize,
			                               /*isSigned=*/false);
		}
		
		/**
		 * \brief Get aggregate for register pair.
		 * 
		 * Given a high and low type that can ideally be used as elements
		 * of a two register pair to pass or return, return a first class
		 * aggregate to represent them.  For example, if the low part of
		 * a by-value argument should be passed as i32* and the high part
		 * as float, return {i32*, float}.
		 */
		Type
		getX86_64ByValArgumentPair(const ABITypeInfo& typeInfo,
		                           Type lowType,
		                           const Type highType) {
			// In order to correctly satisfy the ABI, we need the
			// high part to start at offset 8.  If the high and low
			// parts we inferred are both 4-byte types (e.g. i32 and
			// i32) then the resultant struct type ({i32,i32}) won't
			// have the second element at offset 8.  Check for this:
			const auto lowSize = typeInfo.getTypeAllocSize(lowType);
			const auto highAlign = typeInfo.getTypeRequiredAlign(highType);
			const auto highStart = lowSize.roundUpToAlign(highAlign);
			
			assert(highStart.asBytes() != 0 && highStart.asBytes() <= 8 &&
			       "Invalid x86-64 argument pair!");
			
			// To handle this, we have to increase the size of the
			// low part so that the second element will start at an
			// 8 byte offset.  We can't increase the size of the
			// second element because it might make us access off the
			// end of the struct.
			if (highStart.asBytes() != 8) {
				// There are only two sorts of types the ABI
				// generation code can produce for the low part
				// of a pair that aren't 8 bytes in size: float
				// or i8/i16/i32. Promote these to a larger type.
				if (lowType.isFloat()) {
					lowType = DoubleTy;
				} else {
					assert(lowType.isInteger() &&
					       "Invalid/unknown low type.");
					lowType = Int64Ty;
				}
			}
			
			const auto resultType = typeInfo.typeBuilder().getStructTy({ lowType, highType });
			
			// Verify that the second element is at an 8-byte offset.
			assert(typeInfo.calculateStructOffsets(resultType.structMembers())[1].asBytes() == 8 &&
			       "Invalid x86-64 argument pair!");
			
			return resultType;
		}
		
		Classifier::Classifier(const ABITypeInfo& typeInfo)
		: typeInfo_(typeInfo) { }
		
		Classification Classifier::classify(const Type type) {
			Classification classification;
			
			if (typeInfo_.getTypeAllocSize(type).asBytes() > 32 ||
			    type.hasUnalignedFields(typeInfo_)) {
				// If size exceeds "four eightbytes" or type
				// has "unaligned fields", pass in memory.
				classification.addField(0, Memory);
				return classification;
			}
			
			classification.classifyType(typeInfo_, type, 0);
			
			// If the size of the aggregate exceeds two eightbytes
			// and the first eight-byte isn’t SSE or any other
			// eightbyte isn’t SSEUP, the whole argument is passed
			// in memory.
			if (typeInfo_.getTypeAllocSize(type).asBytes() > 16 &&
			    (classification.low() != Sse ||
			     classification.high() != SseUp)) {
				classification.addField(0, Memory);
			}
			
			return classification;
		}
		
		ArgInfo Classifier::classifyType(const Type type,
		                                 const bool isArgument,
		                                 const unsigned freeIntRegs,
		                                 unsigned& neededInt,
		                                 unsigned& neededSse) {
			// type = useFirstFieldIfTransparentUnion(type);
			
			// AMD64-ABI 3.2.3p4: Rule 1. Classify the return type with the
			// classification algorithm.
			const auto classification = classify(type);
			
			// Sanity check classification.
			assert(classification.high() != Memory ||
			       classification.low() == Memory);
			assert(classification.high() != SseUp ||
			       classification.low() == Sse);
			
			neededInt = 0;
			neededSse = 0;
			
			Type resultType = VoidTy;
			
			switch (classification.low()) {
				case NoClass: {
					if (classification.high() == NoClass) {
						return ArgInfo::getIgnore();
					}
					
					// Low part is just padding.
					assert(classification.high() == Sse ||
					       classification.high() == Integer ||
					       classification.high() == X87Up);
					break;
				}
				case SseUp: {
					llvm_unreachable("Low word can't be SseUp.");
				}
				case X87Up: {
					llvm_unreachable("Low word can't be X87Up.");
				}
				case Memory: {
					if (isArgument) {
						// AMD64-ABI 3.2.3p3: Rule 1. If the class is MEMORY, pass the argument
						// on the stack.
						return getIndirectResult(typeInfo_,
						                         type,
						                         freeIntRegs);
					} else {
						// AMD64-ABI 3.2.3p4: Rule 2. Types of class memory are returned via
						// hidden argument.
						return getIndirectReturnResult(type);
					}
				}
				
				// AMD64-ABI 3.2.3p3: Rule 2. If the class is INTEGER, the next
				// available register of the sequence %rdi, %rsi, %rdx, %rcx, %r8
				// and %r9 is used.
				// AMD64-ABI 3.2.3p4: Rule 3. If the class is INTEGER, the next
				// available register of the sequence %rax, %rdx is used.
				case Integer: {
					++neededInt;
					
					// Pick an 8-byte type based on the preferred type.
					resultType = getINTEGERTypeAtOffset(typeInfo_,
					                                    type, DataSize::Bytes(0),
					                                    type, DataSize::Bytes(0));
					
					// If we have a sign or zero extended integer,
					// make sure to return Extend so that the
					// parameter gets the right LLVM IR attributes.
					if (classification.high() == NoClass &&
					    resultType.isInteger()) {
						if (isIntegralType(type) &&
						    isPromotableIntegerType(type)) {
							return ArgInfo::getExtend(resultType);
						}
					}
					break;
				}
				
				// AMD64-ABI 3.2.3p3: Rule 3. If the class is Sse, the next
				// available Sse register is used, the registers are taken in the
				// order from %xmm0 to %xmm7.
				// AMD64-ABI 3.2.3p4: Rule 4. If the class is Sse, the next
				// available Sse register of the sequence %xmm0, %xmm1 is used.
				case Sse: {
					++neededSse;
					resultType = getSseTypeAtOffset(typeInfo_,
					                                type, DataSize::Bytes(0),
					                                type, DataSize::Bytes(0));
					break;
				}
				case X87: {
					if (isArgument) {
						// AMD64-ABI 3.2.3p3: Rule 5. If the class is X87, X87UP or
						// COMPLEX_X87, it is passed in memory.
						return getIndirectResult(typeInfo_,
						                         type,
						                         freeIntRegs);
					} else {
						// AMD64-ABI 3.2.3p4: Rule 6. If the class is X87, the value is
						// returned on the X87 stack in %st0 as 80-bit x87 number.
						resultType = LongDoubleTy;
					}
					break;
				}
				case ComplexX87: {
					assert(classification.high() == ComplexX87);
					if (isArgument) {
						// AMD64-ABI 3.2.3p3: Rule 5. If the class is X87, X87UP or
						// COMPLEX_X87, it is passed in memory.
						return getIndirectResult(typeInfo_,
						                         type,
						                         freeIntRegs);
					} else {
						// AMD64-ABI 3.2.3p4: Rule 8. If the class is COMPLEX_X87, the real
						// part of the value is returned in %st0 and the imaginary part in
						// %st1.
						resultType = typeInfo_.typeBuilder().getStructTy({ LongDoubleTy, LongDoubleTy });
					}
					break;
				}
			}
			
			Type highPartType = VoidTy;
			
			switch (classification.high()) {
				case Memory: {
					llvm_unreachable("Memory class already handled.");  
				}
				case X87: {
					llvm_unreachable("High word can't be X87.");
				}
				case ComplexX87: {
					llvm_unreachable("High word can't be ComplexX87.");
				}
				case NoClass: {
					// Already handled.
					break;
				}
				case Integer: {
					++neededInt;
					highPartType = getINTEGERTypeAtOffset(typeInfo_,
					                                      type, DataSize::Bytes(8),
					                                      type, DataSize::Bytes(8));
					if (classification.low() == NoClass) {
						// Return high part at offset 8 in memory.
						return ArgInfo::getDirect(highPartType, 8);
					}
					break;
				}
				case Sse: {
					highPartType = getSseTypeAtOffset(typeInfo_,
					                                  type, DataSize::Bytes(8),
					                                  type, DataSize::Bytes(8));
					if (classification.low() == NoClass) {
						// Return high part at offset 8 in memory.
						return ArgInfo::getDirect(highPartType, 8);
					}
					++neededSse;
					break;
				}
				
				// AMD64-ABI 3.2.3p3: Rule 4. If the class is SseUP, the
				// eightbyte is passed in the upper half of the last used Sse
				// register.  This only happens when 128-bit vectors are passed.
				// AMD64-ABI 3.2.3p4: Rule 5. If the class is SseUP, the eightbyte
				// is returned in the next available eightbyte chunk of the last
				// used vector register.
				case SseUp: {
					assert(classification.low() == Sse);
					resultType = getByteVectorType(typeInfo_,
					                               type);
					break;
				}
				
				// AMD64-ABI 3.2.3p4: Rule 7. If the class is X87UP, the value is
				// returned together with the previous X87 value in %st0.
				case X87Up: {
					assert(!isArgument && "TODO");
					if (classification.low() != X87) {
						highPartType = getSseTypeAtOffset(typeInfo_,
						                                  type, DataSize::Bytes(8),
						                                  type, DataSize::Bytes(8));
						if (classification.low() == NoClass) {
							// Return high part at offset 8 in memory.
							return ArgInfo::getDirect(highPartType, 8);
						}
					}
					++neededSse;
					break;
				}
			}
			
			// If a high part was specified, merge it together with the low part.  It is
			// known to pass in the high eightbyte of the result.  We do this by forming a
			// first class struct aggregate with the high and low part: {low, high}
			if (!highPartType.isVoid()) {
				resultType = getX86_64ByValArgumentPair(typeInfo_,
				                                        resultType,
				                                        highPartType);
			}
			
			return ArgInfo::getDirect(resultType);
		}
		
		ArgInfo Classifier::classifyReturnType(const Type type) {
			const bool isArgument = false;
			const unsigned freeIntRegs = 0;
			unsigned neededInt = 0;
			unsigned neededSse = 0;
			return classifyType(type,
			                    isArgument,
			                    freeIntRegs,
			                    neededInt,
			                    neededSse);
		}
		
		llvm::SmallVector<ArgInfo, 8>
		Classifier::classifyFunctionType(const FunctionType& functionType) {
			ArgInfo returnInfo = classifyReturnType(functionType.returnType());
			
			llvm::SmallVector<ArgInfo, 8> argInfoArray;
			argInfoArray.push_back(returnInfo);
			
			// Keep track of the number of assigned registers.
			unsigned freeIntRegs = 6;
			unsigned freeSseRegs = 8;
			
			// If the return value is indirect, then the hidden argument
			// is consuming one integer register.
			if (returnInfo.isIndirect()) {
				--freeIntRegs;
			}
			
			// The chain argument effectively gives us another free register.
			/*if (functionType.isChainCall()) {
				++freeIntRegs;
			}*/
			
			//const unsigned numRequiredArgs = functionType.argumentTypes().size();
			
			// AMD64-ABI 3.2.3p3: Once arguments are classified, the registers
			// get assigned (in left-to-right order) for passing as follows...
			for (size_t i = 0; i < functionType.argumentTypes().size(); i++) {
				//const bool IsNamedArg = i < numRequiredArgs;
				
				const auto argType = functionType.argumentTypes()[i];
				
				const bool isArgument = true;
				unsigned neededInt = 0;
				unsigned neededSse = 0;
				ArgInfo argInfo = classifyType(argType,
				                               isArgument,
				                               freeIntRegs,
				                               neededInt,
				                               neededSse);
				// AMD64-ABI 3.2.3p3: If there are no registers available for any
				// eightbyte of an argument, the whole argument is passed on the
				// stack. If registers have already been assigned for some
				// eightbytes of such an argument, the assignments get reverted.
				if (freeIntRegs >= neededInt && freeSseRegs >= neededSse) {
					freeIntRegs -= neededInt;
					freeSseRegs -= neededSse;
				} else {
					argInfo = getIndirectResult(typeInfo_,
					                            argType,
					                            freeIntRegs);
				}
				
				argInfoArray.push_back(argInfo);
			}
			
			return argInfoArray;
		}
		
	}
	
}
