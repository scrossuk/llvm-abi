#include <cstddef>

#include <llvm/Support/ErrorHandling.h>

#include <llvm-abi/Support.hpp>
#include <llvm-abi/Type.hpp>

#include "ArgClass.hpp"
#include "Classification.hpp"
#include "TypeInfo.hpp"

namespace llvm_abi {
	
	namespace x86_64 {
		
		Classification::Classification() {
			classes[0] = NoClass;
			classes[1] = NoClass;
		}
		
		bool Classification::isMemory() const {
			return classes[0] == Memory;
		}
		
		void Classification::addField(const size_t offset, const ArgClass fieldClass) {
			if (isMemory()) {
				return;
			}
			
			// Note that we don't need to bother checking if it crosses 8 bytes.
			// We don't get here with unaligned fields, and anything that can be
			// big enough to cross 8 bytes (cdoubles, reals, structs and arrays)
			// is special-cased in classifyType()
			const size_t idx = (offset < 8 ? 0 : 1);
			
			const auto mergedClass = mergeClasses(classes[idx], fieldClass);
			
			if (mergedClass != classes[idx]) {
				classes[idx] = mergedClass;
				
				if (mergedClass == Memory) {
					classes[1 - idx] = Memory;
				}
			}
		}
		
		bool hasUnalignedFields(const Type type) {
			if (!type.isStruct()) return false;
			
			size_t offset = 0;
			
			for (const auto& member: type.structMembers()) {
				// Add necessary padding before this member.
				offset = roundUpToAlign(offset, getTypeAlign(member.type()));
				
				const auto memberOffset = member.offset() == 0 ? offset : member.offset();
				
				if (memberOffset != offset || hasUnalignedFields(member.type())) {
					return true;
				}
				
				// Add the member's size.
				offset += getTypeSize(member.type());
			}
			
			return false;
		}
		
		void Classification::classifyType(const Type type, const size_t offset) {
			if (type.isInteger() || type.isPointer()) {
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
					// make sure other half knows about it too:
					addField(offset + 16, ComplexX87);
				}
			} else if (type.isArray()) {
				const auto& elementType = type.arrayElementType();
				const auto elementSize = getTypeSize(elementType);
				
				for (size_t i = 0; i < type.arrayElementCount(); i++) {
					classifyType(elementType, offset + i * elementSize);
				}
			} else if (type.isStruct()) {
				const auto& structMembers = type.structMembers();
				
				size_t structOffset = 0;
				for (const auto& member: structMembers) {
					if (member.offset() < structOffset) {
						// Add necessary padding before this member.
						structOffset = roundUpToAlign(structOffset, getTypeAlign(member.type()));
					} else {
						structOffset = member.offset();
					}
					
					classifyType(member.type(), offset + structOffset);
					
					// Add the member's size.
					structOffset += getTypeSize(member.type());
				}
			} else {
				llvm_unreachable("Unknown type kind.");
			}
		}
		
		Classification classify(const Type type) {
			Classification classification;
			
			if (getTypeSize(type) > 32 || hasUnalignedFields(type)) {
				// If size exceeds "four eightbytes" or type
				// has "unaligned fields", pass in memory.
				classification.addField(0, Memory);
				return classification;
			}
			
			classification.classifyType(type, 0);
			return classification;
		}
		
	}
	
}
