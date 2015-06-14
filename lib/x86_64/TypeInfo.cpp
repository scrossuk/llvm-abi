#include <algorithm>
#include <cstddef>

#include <llvm/Support/ErrorHandling.h>

#include <llvm-abi/Support.hpp>
#include <llvm-abi/Type.hpp>

#include "TypeInfo.hpp"

namespace llvm_abi {
	
	namespace x86_64 {
		
		size_t getTypeSize(const Type* type) {
			switch (type->kind()) {
				case VoidType:
					return 0;
				case PointerType:
					return 8;
				case IntegerType:
					switch (type->integerKind()) {
						case Bool:
						case Char:
						case Int8:
							return 1;
							
						case Short:
						case Int16:
							return 2;
							
						case Int:
						case Int32:
							return 4;
							
						case Long:
						case SizeT:
						case PtrDiffT:
						case LongLong:
						case Int64:
							return 8;
							
						case Int128:
							return 16;
					}
					llvm_unreachable("Unknown integer type.");
				case FloatingPointType:
					switch (type->floatingPointKind()) {
						case Float:
							return 4;
							
						case Double:
							return 8;
							
						case LongDouble:
							return 16;
							
						case Float128:
							return 16;
					}
					llvm_unreachable("Unknown floating point type.");
				case ComplexType:
					switch (type->complexKind()) {
						case Float:
							return 8;
							
						case Double:
							return 16;
							
						case LongDouble:
							return 32;
							
						case Float128:
							return 32;
					}
					llvm_unreachable("Unknown complex type.");
				case StructType: {
					if (type->structMembers().empty()) {
						return getTypeAlign(type);
					}
					
					size_t size = 0;
					
					for (const auto& member: type->structMembers()) {
						if (member.offset() < size) {
							// Add necessary padding before this member.
							size = roundUpToAlign(size, getTypeAlign(member.type()));
						} else {
							size = member.offset();
						}
						
						// Add the member's size.
						size += getTypeSize(member.type());
					}
					
					// Add any final padding.
					return roundUpToAlign(size, getTypeAlign(type));
				}
				case ArrayType:
					// TODO: this is probably wrong...
					return getTypeSize(type->arrayElementType()) * type->arrayElementCount();
			}
			llvm_unreachable("Unknown ABI type.");
		}
		
		size_t getTypeAlign(const Type* type) {
			switch (type->kind()) {
				case StructType: {
					size_t mostStrictAlign = 1;
					for (const auto& member: type->structMembers()) {
						const size_t align = getTypeAlign(member.type());
						mostStrictAlign = std::max<size_t>(mostStrictAlign, align);
					}
					
					return mostStrictAlign;
				}
				case ArrayType: {
					const auto elementAlign = getTypeAlign(type->arrayElementType());
					const size_t minAlign = getTypeSize(type) >= 16 ? 16 : 1;
					return std::max<size_t>(elementAlign, minAlign);
				}
				default:
					return getTypeSize(type);
			}
		}
		
	}
	
}
