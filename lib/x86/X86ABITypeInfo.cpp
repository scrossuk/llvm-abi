#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/DataSize.hpp>
#include <llvm-abi/Type.hpp>

#include "X86ABITypeInfo.hpp"

namespace llvm_abi {
	
	const TypeBuilder& X86ABITypeInfo::typeBuilder() const {
		llvm_unreachable("TODO");
	}
	
	DataSize X86ABITypeInfo::getTypeRawSize(const Type type) const {
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
				llvm_unreachable("TODO");
			}
			case ArrayType: {
				llvm_unreachable("TODO");
			}
			case VectorType: {
				llvm_unreachable("TODO");
			}
		}
		llvm_unreachable("Unknown type kind.");
	}
	
	DataSize X86ABITypeInfo::getTypeAllocSize(const Type type) const {
		// TODO!
		return getTypeRawSize(type);
	}
	
	DataSize X86ABITypeInfo::getTypeStoreSize(const Type type) const {
		// TODO!
		return getTypeAllocSize(type);
	}
	
	DataSize X86ABITypeInfo::getTypeRequiredAlign(const Type type) const {
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
				llvm_unreachable("TODO");
			}
			case ArrayType: {
				llvm_unreachable("TODO");
			}
			case VectorType: {
				llvm_unreachable("TODO");
			}
		}
		llvm_unreachable("Unknown type kind.");
	}
	
	DataSize X86ABITypeInfo::getTypePreferredAlign(const Type type) const {
		// TODO!
		return getTypeRequiredAlign(type);
	}
	
	llvm::Type* X86ABITypeInfo::getLLVMType(const Type type) const {
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
				llvm_unreachable("TODO");
			}
			case ArrayType: {
				llvm_unreachable("TODO");
			}
			case VectorType: {
				llvm_unreachable("TODO");
			}
		}
	}
	
	llvm::SmallVector<DataSize, 8> X86ABITypeInfo::calculateStructOffsets(llvm::ArrayRef<StructMember> /*structMembers*/) const {
		llvm_unreachable("TODO");
	}
	
	bool X86ABITypeInfo::isLegalVectorType(const Type /*type*/) const {
		llvm_unreachable("TODO");
	}
	
	bool X86ABITypeInfo::isBigEndian() const {
		return false;
	}
	
	bool X86ABITypeInfo::isCharSigned() const {
		return true;
	}
	
}

