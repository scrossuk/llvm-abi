#include <cstddef>

#include <llvm/Support/ErrorHandling.h>

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>

#include <llvm-abi/x86_64/ArgClass.hpp>
#include <llvm-abi/x86_64/Classification.hpp>

namespace llvm_abi {
	
	namespace x86_64 {
		
		Classification::Classification() {
			classes_[0] = NoClass;
			classes_[1] = NoClass;
		}
		
		ArgClass Classification::low() const {
			return classes_[0];
		}
		
		ArgClass Classification::high() const {
			return classes_[1];
		}
		
		bool Classification::isMemory() const {
			return classes_[0] == Memory;
		}
		
		void Classification::addField(const size_t offset, const ArgClass fieldClass) {
			if (isMemory()) {
				return;
			}
			
			// Note that we don't need to bother checking if it crosses 8 bytes.
			// We don't get here with unaligned fields, and anything that can be
			// big enough to cross 8 bytes is special-cased in classifyType().
			const size_t idx = (offset < 8 ? 0 : 1);
			
			const auto mergedClass = mergeClasses(classes_[idx], fieldClass);
			
			if (mergedClass != classes_[idx]) {
				classes_[idx] = mergedClass;
				
				if (mergedClass == Memory) {
					classes_[1 - idx] = Memory;
				}
			}
		}
		
		void Classification::classifyType(const ABITypeInfo& typeInfo,
		                                  const Type type,
		                                  const size_t offset) {
			// TODO!
			const bool isNamedArg = true;
			
			if (type.isVoid()) {
				addField(offset, NoClass);
			} else if (type.isInteger() || type.isPointer()) {
				addField(offset, Integer);
			} else if (type.isFloatingPoint()) {
				if (type.floatingPointKind() == LongDouble) {
					addField(offset, X87);
					addField(offset + 8, X87Up);
				} else {
					addField(offset, Sse);
				}
			} else if (type.isComplex()) {
				if (type.complexKind() == Float) {
					addField(offset, Sse);
					addField(offset + 4, Sse);
				} else if (type.complexKind() == Double) {
					addField(offset, Sse);
					addField(offset + 8, Sse);
				} else if (type.complexKind() == LongDouble) {
					addField(offset, ComplexX87);
					addField(offset + 16, ComplexX87);
				}
			} else if (type.isArray()) {
				const auto& elementType = type.arrayElementType();
				const auto elementSize = typeInfo.getTypeAllocSize(elementType);
				
				for (size_t i = 0; i < type.arrayElementCount(); i++) {
					classifyType(typeInfo, elementType, offset + i * elementSize.asBytes());
				}
			} else if (type.isStruct()) {
				const auto& structMembers = type.structMembers();
				
				auto structOffset = DataSize::Bytes(0);
				for (const auto& member: structMembers) {
					if (member.offset() < structOffset) {
						// Add necessary padding before this member.
						structOffset = structOffset.roundUpToAlign(typeInfo.getTypeRequiredAlign(member.type()));
					} else {
						structOffset = member.offset();
					}
					
					classifyType(typeInfo, member.type(), offset + structOffset.asBytes());
					
					// Add the member's size.
					structOffset += typeInfo.getTypeAllocSize(member.type());
				}
			} else if (type.isVector()) {
				const auto size = typeInfo.getTypeAllocSize(type);
				const auto elementType = type.vectorElementType();
				
				if (size.asBits() == 32) {
					// gcc passes all <4 x char>, <2 x short>,
					// <1 x int>, <1 x float> as integer.
					addField(offset, Integer);
				} else if (size.asBits() == 64) {
					// gcc passes <1 x double> in memory. :(
					if (elementType == DoubleTy) {
						addField(offset, Memory);
						return;
					}
					
					// gcc passes <1 x long long> as INTEGER.
					if (elementType == LongLongTy ||
					    elementType == ULongLongTy ||
					    elementType == LongTy ||
					    elementType == ULongTy) {
						addField(offset, Integer);
					} else {
						addField(offset, Sse);
					}
				} else if (size.asBits() == 128 ||
					   (isNamedArg && size.asBits() == 256 && typeInfo.isLegalVectorType(type))) {
					// Arguments of 256-bits are split into four eightbyte chunks. The
					// least significant one belongs to class SSE and all the others to class
					// SSEUP. The original Lo and Hi design considers that types can't be
					// greater than 128-bits, so a 64-bit split in Hi and Lo makes sense.
					// This design isn't correct for 256-bits, but since there're no cases
					// where the upper parts would need to be inspected, avoid adding
					// complexity and just consider Hi to match the 64-256 part.
					//
					// Note that per 3.5.7 of AMD64-ABI, 256-bit args are only passed in
					// registers if they are "named", i.e. not part of the "..." of a
					// variadic function.
					addField(offset, Sse);
					addField(offset + 8, SseUp);
				}
			} else {
				llvm_unreachable("Unknown type kind.");
			}
		}
		
	}
	
}
