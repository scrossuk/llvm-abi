#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/DataSize.hpp>
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
		
		DataSize X86_64ABITypeInfo::getTypeRawSize(const Type type) const {
			switch (type.kind()) {
				case VoidType:
					return DataSize::Bytes(0);
				case PointerType:
					return DataSize::Bytes(8);
				case IntegerType:
					switch (type.integerKind()) {
						case Bool:
						case Char:
						case Int8:
							return DataSize::Bytes(1);
						case Short:
						case Int16:
							return DataSize::Bytes(2);
						case Int24:
							return DataSize::Bytes(3);
						case Int:
						case Int32:
							return DataSize::Bytes(4);
						case Long:
						case SizeT:
						case PtrDiffT:
						case LongLong:
						case Int64:
							return DataSize::Bytes(8);
						case Int128:
							return DataSize::Bytes(16);
					}
					llvm_unreachable("Unknown integer type.");
				case FloatingPointType:
					switch (type.floatingPointKind()) {
						case Float:
							return DataSize::Bytes(4);
						case Double:
							return DataSize::Bytes(8);
						case LongDouble:
							return DataSize::Bytes(16);
						case Float128:
							return DataSize::Bytes(16);
					}
					llvm_unreachable("Unknown floating point type.");
				case ComplexType:
					switch (type.complexKind()) {
						case Float:
							return DataSize::Bytes(8);
							
						case Double:
							return DataSize::Bytes(16);
							
						case LongDouble:
							return DataSize::Bytes(32);
							
						case Float128:
							return DataSize::Bytes(32);
					}
					llvm_unreachable("Unknown complex type.");
				case StructType: {
					if (type.structMembers().empty()) {
						return DataSize::Bytes(0);
					}
					
					auto size = DataSize::Bytes(0);
					
					for (const auto& member: type.structMembers()) {
						if (member.offset() < size) {
							// Add necessary padding before this member.
							size = size.roundUpToAlign(getTypeRequiredAlign(member.type()));
						} else {
							size = member.offset();
						}
						
						// Add the member's size.
						size += getTypeAllocSize(member.type());
					}
					
					// Add any final padding.
					return size.roundUpToAlign(getTypeRequiredAlign(type));
				}
				case ArrayType:
					// TODO: this is probably wrong...
					return getTypeRawSize(type.arrayElementType()) * type.arrayElementCount();
				case VectorType:
					// TODO: this is probably wrong...
					return getTypeRawSize(type.vectorElementType()) * type.vectorElementCount();
			}
			llvm_unreachable("Unknown ABI type.");
		}
		
		DataSize X86_64ABITypeInfo::getTypeAllocSize(const Type type) const {
			if (type.isInteger() && type.integerKind() == Int24) {
				return DataSize::Bytes(4);
			}
		}
		
		DataSize X86_64ABITypeInfo::getTypeStoreSize(const Type type) const {
			return getTypeAllocSize(type);
		}
		
		DataSize X86_64ABITypeInfo::getTypeRequiredAlign(const Type type) const {
			switch (type.kind()) {
				case StructType: {
					auto mostStrictAlign = DataSize::Bytes(1);
					for (const auto& member: type.structMembers()) {
						const auto align = getTypeRequiredAlign(member.type());
						mostStrictAlign = std::max<DataSize>(mostStrictAlign, align);
					}
					
					return mostStrictAlign;
				}
				case ArrayType: {
					const auto elementAlign = getTypeRequiredAlign(type.arrayElementType());
					const auto minAlign = getTypeAllocSize(type).asBytes() >= 16 ? DataSize::Bytes(16) : DataSize::Bytes(1);
					return std::max<DataSize>(elementAlign, minAlign);
				}
				case VectorType: {
					const auto elementAlign = getTypeRequiredAlign(type.vectorElementType());
					const auto minAlign = getTypeAllocSize(type).asBytes() >= 16 ? DataSize::Bytes(16) : DataSize::Bytes(1);
					return std::max<DataSize>(elementAlign, minAlign);
				}
				default:
					return getTypeAllocSize(type);
			}
		}
		
		DataSize X86_64ABITypeInfo::getTypePreferredAlign(const Type type) const {
			return getTypeRequiredAlign(type);
		}
		
		llvm::Type* X86_64ABITypeInfo::getLLVMType(const Type type) const  {
			switch (type.kind()) {
				case VoidType:
					return llvm::Type::getVoidTy(llvmContext_);
				case PointerType:
					return llvm::Type::getInt8PtrTy(llvmContext_);
				case IntegerType: {
					return llvm::IntegerType::get(llvmContext_, getTypeRawSize(type).asBits());
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
		
		llvm::SmallVector<DataSize, 8> X86_64ABITypeInfo::calculateStructOffsets(llvm::ArrayRef<StructMember> structMembers) const {
			llvm::SmallVector<DataSize, 8> offsets;
			offsets.reserve(structMembers.size());
			
			auto offset = DataSize::Bytes(0);
			for (const auto& member: structMembers) {
				if (member.offset() < offset) {
					// Add necessary padding before this member.
					offset = offset.roundUpToAlign(getTypeRequiredAlign(member.type()));
				} else {
					offset = member.offset();
				}
				
				offsets.push_back(offset);
				
				// Add the member's size.
				offset += getTypeAllocSize(member.type());
			}
			
			return offsets;
		}
		
		bool X86_64ABITypeInfo::isCharSigned() const {
			return true;
		}
		
	}
	
}
