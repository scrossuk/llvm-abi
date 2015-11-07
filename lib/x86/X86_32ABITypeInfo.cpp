#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/DataSize.hpp>
#include <llvm-abi/DefaultABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>

#include <llvm-abi/x86/X86_32ABITypeInfo.hpp>

namespace llvm_abi {
	
	namespace x86 {
		
		X86_32ABITypeInfo::X86_32ABITypeInfo(llvm::LLVMContext& llvmContext)
		: llvmContext_(llvmContext),
		defaultABITypeInfo_(llvmContext, /*typeInfo=*/*this,
		                    /*delegate=*/*this)
		{ }
		
		const TypeBuilder& X86_32ABITypeInfo::typeBuilder() const {
			return typeBuilder_;
		}
		
		DataSize X86_32ABITypeInfo::getTypeRawSize(const Type type) const {
			return defaultABITypeInfo_.getDefaultTypeRawSize(type);
		}
		
		DataSize X86_32ABITypeInfo::getTypeAllocSize(const Type type) const {
			return defaultABITypeInfo_.getDefaultTypeAllocSize(type);
		}
		
		DataSize X86_32ABITypeInfo::getTypeStoreSize(const Type type) const {
			return defaultABITypeInfo_.getDefaultTypeStoreSize(type);
		}
		
		DataSize X86_32ABITypeInfo::getTypeRequiredAlign(const Type type) const {
			return defaultABITypeInfo_.getDefaultTypeRequiredAlign(type);
		}
		
		DataSize X86_32ABITypeInfo::getTypePreferredAlign(const Type type) const {
			return defaultABITypeInfo_.getDefaultTypePreferredAlign(type);
		}
		
		llvm::Type* X86_32ABITypeInfo::getLLVMType(const Type type) const {
			return defaultABITypeInfo_.getDefaultLLVMType(type);
		}
		
		llvm::SmallVector<DataSize, 8>
		X86_32ABITypeInfo::calculateStructOffsets(llvm::ArrayRef<RecordMember> structMembers) const {
			return defaultABITypeInfo_.calculateDefaultStructOffsets(structMembers);
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
		
		DataSize X86_32ABITypeInfo::getPointerSize() const {
			return DataSize::Bytes(4);
		}
		
		DataSize X86_32ABITypeInfo::getPointerAlign() const {
			return DataSize::Bytes(4);
		}
		
		DataSize X86_32ABITypeInfo::getIntSize(const IntegerKind kind) const {
			switch (kind) {
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
		
		DataSize X86_32ABITypeInfo::getIntAlign(const IntegerKind kind) const {
			switch (kind) {
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
		
		DataSize X86_32ABITypeInfo::getFloatSize(const FloatingPointKind kind) const {
			switch (kind) {
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
		
		DataSize X86_32ABITypeInfo::getFloatAlign(const FloatingPointKind kind) const {
			switch (kind) {
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
		
		DataSize X86_32ABITypeInfo::getComplexSize(const FloatingPointKind kind) const {
			switch (kind) {
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
		
		DataSize X86_32ABITypeInfo::getComplexAlign(const FloatingPointKind kind) const {
			switch (kind) {
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
		
		static DataSize getVectorMinAlign(const DataSize size) {
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
		
		DataSize X86_32ABITypeInfo::getArrayAlign(const Type type) const {
			return getTypeRequiredAlign(type.arrayElementType());
		}
		
		DataSize X86_32ABITypeInfo::getVectorAlign(const Type type) const {
			const auto elementAlign = getTypeRequiredAlign(type.vectorElementType());
			const auto minAlign = getVectorMinAlign(getTypeAllocSize(type));
			return std::max<DataSize>(elementAlign, minAlign);
		}
		
		llvm::Type* X86_32ABITypeInfo::getLongDoubleIRType() const {
			return llvm::Type::getX86_FP80Ty(llvmContext_);
		}
		
	}
	
}

