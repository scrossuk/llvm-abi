#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/DataSize.hpp>
#include <llvm-abi/Type.hpp>

#include <llvm-abi/x86/X86_32ABITypeInfo.hpp>

namespace llvm_abi {
	
	namespace x86 {
		
		const TypeBuilder& X86_32ABITypeInfo::typeBuilder() const {
			llvm_unreachable("TODO");
		}
		
		DataSize X86_32ABITypeInfo::getTypeRawSize(const Type type) const {
			switch (type.kind()) {
				case VoidType:
					return DataSize::Bytes(0);
				case PointerType:
					return DataSize::Bytes(4);
				case UnspecifiedWidthIntegerType: {
					switch (type.integerKind()) {
						case Bool:
							return DataSize::Bytes(1);
						case Char:
						case SChar:
						case UChar:
							return DataSize::Bytes(1);
						case Short:
						case UShort:
							return DataSize::Bytes(2);
						case Int:
						case UInt:
							return DataSize::Bytes(4);
						case Long:
						case ULong:
							return DataSize::Bytes(4);
						case LongLong:
						case ULongLong:
							return DataSize::Bytes(8);
						case SizeT:
						case SSizeT:
							return DataSize::Bytes(4);
						case IntPtrT:
						case UIntPtrT:
						case PtrDiffT:
							return DataSize::Bytes(4);
					}
					llvm_unreachable("Unknown Integer type kind.");
				}
				case FixedWidthIntegerType: {
					return type.integerWidth().roundUpToPowerOf2Bytes();
				}
				case FloatingPointType: {
					switch (type.floatingPointKind()) {
						case HalfFloat:
							llvm_unreachable("TODO");
						case Float:
							return DataSize::Bytes(4);
						case Double:
							return DataSize::Bytes(8);
						case LongDouble:
							// NB: Apparently on Android this is the same as 'double'.
							return DataSize::Bytes(12);
						case Float128:
							return DataSize::Bytes(16);
					}
					llvm_unreachable("Unknown Float type kind.");
				}
				case ComplexType: {
					switch (type.complexKind()) {
						case HalfFloat:
							llvm_unreachable("TODO");
						case Float:
							return DataSize::Bytes(8);
						case Double:
							return DataSize::Bytes(16);
						case LongDouble:
							// NB: Apparently on Android this is the same as 'double'.
							return DataSize::Bytes(24);
						case Float128:
							return DataSize::Bytes(32);
					}
					llvm_unreachable("Unknown Complex type kind.");
				}
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
				case UnionType: {
					auto size = DataSize::Bytes(0);
					
					for (const auto& member: type.unionMembers()) {
						const auto memberSize = getTypeAllocSize(member);
						size = std::max<DataSize>(size, memberSize);
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
			llvm_unreachable("Unknown type kind.");
		}
		
		DataSize X86_32ABITypeInfo::getTypeAllocSize(const Type type) const {
			// TODO!
			return getTypeRawSize(type);
		}
		
		DataSize X86_32ABITypeInfo::getTypeStoreSize(const Type type) const {
			// TODO!
			return getTypeAllocSize(type);
		}
		
		DataSize getVectorMinAlign(const DataSize size) {
			if (size.asBytes() >= 32) {
				return DataSize::Bytes(32);
			} else if (size.asBytes() >= 16) {
				return DataSize::Bytes(16);
			} else if (size.asBytes() >= 8) {
				return DataSize::Bytes(8);
			} else if (size.asBytes() >= 4) {
				return DataSize::Bytes(4);
			} else {
				return DataSize::Bytes(1);
			}
		}
		
		DataSize X86_32ABITypeInfo::getTypeRequiredAlign(const Type type) const {
			switch (type.kind()) {
				case VoidType:
					return DataSize::Bytes(0);
				case PointerType:
					return DataSize::Bytes(4);
				case UnspecifiedWidthIntegerType: {
					switch (type.integerKind()) {
						case Bool:
							return DataSize::Bytes(1);
						case Char:
						case SChar:
						case UChar:
							return DataSize::Bytes(1);
						case Short:
						case UShort:
							return DataSize::Bytes(2);
						case Int:
						case UInt:
							return DataSize::Bytes(4);
						case Long:
						case ULong:
							return DataSize::Bytes(4);
						case LongLong:
						case ULongLong:
							return DataSize::Bytes(4);
						case SizeT:
						case SSizeT:
							return DataSize::Bytes(4);
						case IntPtrT:
						case UIntPtrT:
						case PtrDiffT:
							return DataSize::Bytes(4);
					}
					llvm_unreachable("Unknown Integer type kind.");
				}
				case FixedWidthIntegerType: {
					return type.integerWidth().roundUpToPowerOf2Bytes();
				}
				case FloatingPointType: {
					switch (type.floatingPointKind()) {
						case HalfFloat:
							llvm_unreachable("TODO");
						case Float:
							return DataSize::Bytes(4);
						case Double:
							return DataSize::Bytes(4);
						case LongDouble:
							return DataSize::Bytes(4);
						case Float128:
							return DataSize::Bytes(16);
					}
					llvm_unreachable("Unknown Float type kind.");
				}
				case ComplexType: {
					switch (type.complexKind()) {
						case HalfFloat:
							llvm_unreachable("TODO");
						case Float:
							return DataSize::Bytes(4);
						case Double:
							return DataSize::Bytes(4);
						case LongDouble:
							return DataSize::Bytes(4);
						case Float128:
							return DataSize::Bytes(16);
					}
					llvm_unreachable("Unknown Complex type kind.");
				}
				case StructType: {
					auto mostStrictAlign = DataSize::Bytes(1);
					for (const auto& member: type.structMembers()) {
						const auto align = getTypeRequiredAlign(member.type());
						mostStrictAlign = std::max<DataSize>(mostStrictAlign, align);
					}
					
					return mostStrictAlign;
				}
				case UnionType: {
					auto mostStrictAlign = DataSize::Bytes(1);
					for (const auto& member: type.unionMembers()) {
						const auto align = getTypeRequiredAlign(member);
						mostStrictAlign = std::max<DataSize>(mostStrictAlign, align);
					}
					
					return mostStrictAlign;
				}
				case ArrayType: {
					return getTypeRequiredAlign(type.arrayElementType());
				}
				case VectorType: {
					const auto elementAlign = getTypeRequiredAlign(type.vectorElementType());
					const auto minAlign = getVectorMinAlign(getTypeAllocSize(type));
					return std::max<DataSize>(elementAlign, minAlign);
				}
			}
			llvm_unreachable("Unknown type kind.");
		}
		
		DataSize X86_32ABITypeInfo::getTypePreferredAlign(const Type type) const {
			// TODO!
			return getTypeRequiredAlign(type);
		}
		
		llvm::Type* X86_32ABITypeInfo::getLLVMType(const Type type) const {
			switch (type.kind()) {
				case VoidType:
					return llvm::Type::getVoidTy(llvmContext_);
				case PointerType:
					return llvm::Type::getInt8PtrTy(llvmContext_);
				case UnspecifiedWidthIntegerType:
				case FixedWidthIntegerType: {
					return llvm::IntegerType::get(llvmContext_, getTypeRawSize(type).asBits());
				}
				case FloatingPointType: {
					switch (type.floatingPointKind()) {
						case HalfFloat:
							llvm_unreachable("TODO");
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
				case UnionType: {
					auto maxSize = DataSize::Bytes(0);
					llvm::Type* maxSizeLLVMType = llvm::Type::getInt8Ty(llvmContext_);
					for (const auto& member: type.unionMembers()) {
						const auto size = getTypeAllocSize(member);
						if (size > maxSize) {
							maxSize = size;
							maxSizeLLVMType = getLLVMType(member);
						}
					}
					return llvm::StructType::get(maxSizeLLVMType, nullptr);
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
			llvm_unreachable("Unknown type kind.");
		}
		
		llvm::SmallVector<DataSize, 8> X86_32ABITypeInfo::calculateStructOffsets(llvm::ArrayRef<StructMember> /*structMembers*/) const {
			llvm_unreachable("TODO");
		}
		
		bool X86_32ABITypeInfo::isLegalVectorType(const Type /*type*/) const {
			llvm_unreachable("TODO");
		}
		
		bool X86_32ABITypeInfo::isBigEndian() const {
			return false;
		}
		
		bool X86_32ABITypeInfo::isCharSigned() const {
			return true;
		}
		
		/// Returns true if this type can be passed in SSE registers with the
		/// X86_VectorCall calling convention. Shared between x86_32 and x86_64.
		static bool isX86VectorTypeForVectorCall(const ABITypeInfo& typeInfo,
		                                         const Type type) {
			if (type.isFloatingPoint()) {
				return type != HalfFloatTy;
			} else if (type.isVector()) {
				// vectorcall can pass XMM, YMM, and ZMM vectors.
				// We don't pass SSE1 MMX registers specially.
				const auto size = typeInfo.getTypeAllocSize(type);
				return size.asBytes() == 128 &&
				       size.asBytes() == 256 &&
				       size.asBytes() == 512;
			}
			return false;
		}
		
		/// Returns true if this aggregate is small enough to be passed in SSE registers
		/// in the X86_VectorCall calling convention. Shared between x86_32 and x86_64.
		static bool isX86VectorCallAggregateSmallEnough(const uint64_t numMembers) {
			return numMembers <= 4;
		}
		
		bool X86_32ABITypeInfo::isHomogeneousAggregateBaseType(const Type type) const {
			// FIXME: Assumes vectorcall is in use.
			return isX86VectorTypeForVectorCall(*this, type);
		}
		
		bool X86_32ABITypeInfo::isHomogeneousAggregateSmallEnough(const Type /*base*/,
		                                                          const uint64_t members) const {
			// FIXME: Assumes vectorcall is in use.
			return isX86VectorCallAggregateSmallEnough(members);
		}
		
	}
	
}

