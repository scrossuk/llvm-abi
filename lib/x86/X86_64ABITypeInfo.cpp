#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/DataSize.hpp>
#include <llvm-abi/DefaultABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <llvm-abi/x86/CPUFeatures.hpp>
#include <llvm-abi/x86/X86_64ABITypeInfo.hpp>

namespace llvm_abi {
	
	namespace x86 {
		
		X86_64ABITypeInfo::X86_64ABITypeInfo(llvm::LLVMContext& llvmContext,
		                                     const CPUFeatures& cpuFeatures)
		: llvmContext_(llvmContext),
		cpuFeatures_(cpuFeatures),
		defaultABITypeInfo_(llvmContext, /*typeInfo=*/*this,
		                    /*delegate=*/*this)
		{ }
		
		const TypeBuilder& X86_64ABITypeInfo::typeBuilder() const {
			return typeBuilder_;
		}
		
		DataSize X86_64ABITypeInfo::getTypeRawSize(const Type type) const {
			return defaultABITypeInfo_.getDefaultTypeRawSize(type);
		}
		
		DataSize X86_64ABITypeInfo::getTypeAllocSize(const Type type) const {
			return defaultABITypeInfo_.getDefaultTypeAllocSize(type);
		}
		
		DataSize X86_64ABITypeInfo::getTypeStoreSize(const Type type) const {
			return defaultABITypeInfo_.getDefaultTypeStoreSize(type);
		}
		
		DataSize X86_64ABITypeInfo::getTypeRequiredAlign(const Type type) const {
			return defaultABITypeInfo_.getDefaultTypeRequiredAlign(type);
		}
		
		DataSize X86_64ABITypeInfo::getTypePreferredAlign(const Type type) const {
			return defaultABITypeInfo_.getDefaultTypePreferredAlign(type);
		}
		
		llvm::Type* X86_64ABITypeInfo::getLLVMType(const Type type) const  {
			return defaultABITypeInfo_.getDefaultLLVMType(type);
		}
		
		llvm::SmallVector<DataSize, 8>
		X86_64ABITypeInfo::calculateStructOffsets(llvm::ArrayRef<RecordMember> structMembers) const {
			return defaultABITypeInfo_.calculateDefaultStructOffsets(structMembers);
		}
		
		bool X86_64ABITypeInfo::isLegalVectorType(const Type type) const {
			assert(type.isVector());
			const auto size = getTypeAllocSize(type);
			const size_t largestVector = cpuFeatures_.hasAVX() ? 256 : 128;
			return size.asBits() > 64 && size.asBits() <= largestVector;
		}
		
		bool X86_64ABITypeInfo::isBigEndian() const {
			return false;
		}
		
		bool X86_64ABITypeInfo::isCharSigned() const {
			return true;
		}
		
		DataSize X86_64ABITypeInfo::getPointerSize() const {
			return DataSize::Bytes(8);
		}
		
		DataSize X86_64ABITypeInfo::getPointerAlign() const {
			return DataSize::Bytes(8);
		}
		
		DataSize X86_64ABITypeInfo::getIntSize(const IntegerKind kind) const {
			switch (kind) {
				case Bool:
				case Char:
				case UChar:
				case SChar:
					return DataSize::Bytes(1);
				case Short:
				case UShort:
					return DataSize::Bytes(2);
				case Int:
				case UInt:
					return DataSize::Bytes(4);
				case Long:
				case ULong:
				case SizeT:
				case SSizeT:
				case PtrDiffT:
				case IntPtrT:
				case UIntPtrT:
				case LongLong:
				case ULongLong:
					return DataSize::Bytes(8);
			}
			llvm_unreachable("Unknown Integer type kind.");
		}
		
		DataSize X86_64ABITypeInfo::getIntAlign(const IntegerKind kind) const {
			switch (kind) {
				case Bool:
				case Char:
				case UChar:
				case SChar:
					return DataSize::Bytes(1);
				case Short:
				case UShort:
					return DataSize::Bytes(2);
				case Int:
				case UInt:
					return DataSize::Bytes(4);
				case Long:
				case ULong:
				case SizeT:
				case SSizeT:
				case PtrDiffT:
				case IntPtrT:
				case UIntPtrT:
				case LongLong:
				case ULongLong:
					return DataSize::Bytes(8);
			}
			llvm_unreachable("Unknown Integer type kind.");
		}
		
		DataSize X86_64ABITypeInfo::getFloatSize(const FloatingPointKind kind) const {
			switch (kind) {
				case HalfFloat:
					llvm_unreachable("TODO");
				case Float:
					return DataSize::Bytes(4);
				case Double:
					return DataSize::Bytes(8);
				case LongDouble:
					return DataSize::Bytes(16);
				case Float128:
					return DataSize::Bytes(16);
			}
			llvm_unreachable("Unknown Float type kind.");
		}
		
		DataSize X86_64ABITypeInfo::getFloatAlign(const FloatingPointKind kind) const {
			switch (kind) {
				case HalfFloat:
					llvm_unreachable("TODO");
				case Float:
					return DataSize::Bytes(4);
				case Double:
					return DataSize::Bytes(8);
				case LongDouble:
					return DataSize::Bytes(16);
				case Float128:
					return DataSize::Bytes(16);
			}
			llvm_unreachable("Unknown Float type kind.");
		}
		
		DataSize X86_64ABITypeInfo::getComplexSize(const FloatingPointKind kind) const {
			switch (kind) {
				case HalfFloat:
					llvm_unreachable("TODO");
				case Float:
					return DataSize::Bytes(8);
					
				case Double:
					return DataSize::Bytes(16);
					
				case LongDouble:
					return DataSize::Bytes(32);
					
				case Float128:
					return DataSize::Bytes(32);
			}
			llvm_unreachable("Unknown Complex type kind.");
		}
		
		DataSize X86_64ABITypeInfo::getComplexAlign(const FloatingPointKind kind) const {
			switch (kind) {
				case HalfFloat:
					llvm_unreachable("TODO");
				case Float:
					return DataSize::Bytes(8);
					
				case Double:
					return DataSize::Bytes(16);
					
				case LongDouble:
					return DataSize::Bytes(32);
					
				case Float128:
					return DataSize::Bytes(32);
			}
			llvm_unreachable("Unknown Complex type kind.");
		}
		
		DataSize X86_64ABITypeInfo::getArrayAlign(const Type type) const {
			const auto elementAlign = getTypeRequiredAlign(type.arrayElementType());
			
			// AMD64-ABI 3.1.2p3: An array uses the
			// same alignment as its elements, except
			// that a local or global array variable
			// of length at least 16 bytes or a C99
			// variable-length array variable always
			// has alignment of at least 16 bytes.
			const auto minAlign = getTypeAllocSize(type).asBytes() >= 16 ? DataSize::Bytes(16) : DataSize::Bytes(1);
			return std::max<DataSize>(elementAlign, minAlign);
		}
		
		DataSize X86_64ABITypeInfo::getVectorAlign(const Type type) const {
			const auto elementAlign = getTypeRequiredAlign(type.vectorElementType());
			const auto minAlign =
				getTypeAllocSize(type).asBytes() >= 32 ?
					DataSize::Bytes(32) :
					getTypeAllocSize(type).asBytes() >= 16 ?
						DataSize::Bytes(16) :
						DataSize::Bytes(1);
			return std::max<DataSize>(elementAlign, minAlign);
		}
		
		llvm::Type* X86_64ABITypeInfo::getLongDoubleIRType() const {
			return llvm::Type::getX86_FP80Ty(llvmContext_);
		}
		
	}
	
}
