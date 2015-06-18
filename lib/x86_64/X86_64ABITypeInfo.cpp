#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Support.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <llvm-abi/x86_64/X86_64ABITypeInfo.hpp>

namespace llvm_abi {
	
	namespace x86_64 {
		
		X86_64ABITypeInfo::X86_64ABITypeInfo(llvm::LLVMContext& llvmContext)
		: llvmContext_(llvmContext) { }
		
		const TypeBuilder& X86_64ABITypeInfo::typeBuilder() const {
			return typeBuilder_;
		}
		
		size_t X86_64ABITypeInfo::getTypeSize(const Type type) const {
			switch (type.kind()) {
				case VoidType:
					return 0;
				case PointerType:
					return 8;
				case IntegerType:
					switch (type.integerKind()) {
						case Bool:
						case Char:
						case Int8:
							return 1;
							
						case Short:
						case Int16:
							return 2;
						
						case Int24:
							return 3;
						
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
					switch (type.floatingPointKind()) {
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
					switch (type.complexKind()) {
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
					if (type.structMembers().empty()) {
						return getTypeAlign(type);
					}
					
					size_t size = 0;
					
					for (const auto& member: type.structMembers()) {
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
					return getTypeSize(type.arrayElementType()) * type.arrayElementCount();
				case VectorType:
					// TODO: this is probably wrong...
					return getTypeSize(type.vectorElementType()) * type.vectorElementCount();
			}
			llvm_unreachable("Unknown ABI type.");
		}
		
		size_t X86_64ABITypeInfo::getTypeAlign(const Type type) const {
			if (type.isInteger() && type.integerKind() == Int24) {
				return 4;
			}
			
			switch (type.kind()) {
				case StructType: {
					size_t mostStrictAlign = 1;
					for (const auto& member: type.structMembers()) {
						const size_t align = getTypeAlign(member.type());
						mostStrictAlign = std::max<size_t>(mostStrictAlign, align);
					}
					
					return mostStrictAlign;
				}
				case ArrayType: {
					const auto elementAlign = getTypeAlign(type.arrayElementType());
					const size_t minAlign = getTypeSize(type) >= 16 ? 16 : 1;
					return std::max<size_t>(elementAlign, minAlign);
				}
				case VectorType: {
					const auto elementAlign = getTypeAlign(type.vectorElementType());
					const size_t minAlign = getTypeSize(type) >= 16 ? 16 : 1;
					return std::max<size_t>(elementAlign, minAlign);
				}
				default:
					return getTypeSize(type);
			}
		}
		
		llvm::Type* X86_64ABITypeInfo::getLLVMType(const Type type) const  {
			switch (type.kind()) {
				case VoidType:
					return llvm::Type::getVoidTy(llvmContext_);
				case PointerType:
					return llvm::Type::getInt8PtrTy(llvmContext_);
				case IntegerType: {
					return llvm::IntegerType::get(llvmContext_, getTypeSize(type) * 8);
				}
				case FloatingPointType: {
					switch (type.floatingPointKind()) {
						case Float:
							return llvm::Type::getFloatTy(llvmContext_);
						case Double:
							return llvm::Type::getDoubleTy(llvmContext_);
						case LongDouble:
							return llvm::Type::getX86_FP80Ty(llvmContext_);
						case Float128:
							return llvm::Type::getFP128Ty(llvmContext_);
					}
					llvm_unreachable("Unknown Float type kind.");
				}
				case ComplexType: {
					llvm_unreachable("TODO");
				}
				case StructType: {
					llvm::SmallVector<llvm::Type*, 8> members;
					for (const auto& structMember: type.structMembers()) {
						members.push_back(getLLVMType(structMember.type()));
					}
					return llvm::StructType::get(llvmContext_, members);
				}
				case ArrayType: {
					return llvm::ArrayType::get(getLLVMType(type.arrayElementType()),
					                            type.arrayElementCount());
				}
				case VectorType: {
					return llvm::VectorType::get(getLLVMType(type.vectorElementType()),
					                            type.vectorElementCount());
				}
			}
			
			llvm_unreachable("Unknown type for creating IR type.");
		}
		
		llvm::SmallVector<size_t, 8> X86_64ABITypeInfo::calculateStructOffsets(llvm::ArrayRef<StructMember> structMembers) const {
			llvm::SmallVector<size_t, 8> offsets;
			offsets.reserve(structMembers.size());
			
			size_t offset = 0;
			for (const auto& member: structMembers) {
				if (member.offset() < offset) {
					// Add necessary padding before this member.
					offset = roundUpToAlign(offset, getTypeAlign(member.type()));
				} else {
					offset = member.offset();
				}
				
				offsets.push_back(offset);
				
				// Add the member's size.
				offset += getTypeSize(member.type());
			}
			
			return offsets;
		}
		
		bool X86_64ABITypeInfo::isCharSigned() const {
			return true;
		}
		
	}
	
}
