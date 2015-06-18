#include <cstddef>

#include <llvm/Support/ErrorHandling.h>

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Support.hpp>
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
				const auto elementSize = typeInfo.getTypeSize(elementType);
				
				for (size_t i = 0; i < type.arrayElementCount(); i++) {
					classifyType(typeInfo, elementType, offset + i * elementSize);
				}
			} else if (type.isStruct()) {
				const auto& structMembers = type.structMembers();
				
				size_t structOffset = 0;
				for (const auto& member: structMembers) {
					if (member.offset() < structOffset) {
						// Add necessary padding before this member.
						structOffset = roundUpToAlign(structOffset, typeInfo.getTypeAlign(member.type()));
					} else {
						structOffset = member.offset();
					}
					
					classifyType(typeInfo, member.type(), offset + structOffset);
					
					// Add the member's size.
					structOffset += typeInfo.getTypeSize(member.type());
				}
			} else {
				llvm_unreachable("Unknown type kind.");
			}
		}
		
	}
	
}
