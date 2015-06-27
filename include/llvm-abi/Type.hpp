#ifndef LLVMABI_ABITYPE_HPP
#define LLVMABI_ABITYPE_HPP

#include <string>
#include <vector>

#include <llvm/ADT/ArrayRef.h>

#include <llvm-abi/DataSize.hpp>

namespace llvm_abi {
	
	/**
	 * \brief Integer Kind
	 * 
	 * A set of possible integer types.
	 */
	enum IntegerKind {
		Bool,
		Char,
		SChar,
		UChar,
		Short,
		UShort,
		Int,
		UInt,
		Long,
		ULong,
		LongLong,
		ULongLong,
		SizeT,
		SSizeT,
		PtrDiffT,
		IntPtrT,
		UIntPtrT
	};
	
	/**
	 * \brief Floating Point Kind
	 * 
	 * A set of possible floating point types.
	 */
	enum FloatingPointKind {
		Float,
		Double,
		LongDouble,
		Float128
	};
	
	/**
	 * \brief Type Kind
	 * 
	 * A set of possible type kinds.
	 */
	enum TypeKind {
		VoidType,
		PointerType,
		UnspecifiedWidthIntegerType,
		FixedWidthIntegerType,
		FloatingPointType,
		ComplexType,
		StructType,
		ArrayType,
		VectorType
	};
	
	// Forward declaration.
	class ABITypeInfo;
	class StructMember;
	class TypeBuilder;
	
	/**
	 * \brief ABI Type
	 * 
	 * A representation of a C type for the purpose
	 * of ABI-compliant code generation.
	 */
	class Type {
		public:
			/**
			 * \brief Void Type
			 */
			static Type Void();
			
			/**
			 * \brief Integer Type
			 */
			static Type Pointer();
			
			/**
			 * \brief Integer Type
			 */
			static Type UnspecifiedWidthInteger(IntegerKind kind);
			
			/**
			 * \brief Fixed Size Integer Type
			 */
			static Type FixedWidthInteger(DataSize width, bool isSigned);
			
			/**
			 * \brief Floating Point Type
			 */
			static Type FloatingPoint(FloatingPointKind kind);
			
			/**
			 * \brief Complex Type
			 */
			static Type Complex(FloatingPointKind kind);
			
			/**
			 * \brief Struct Type
			 */
			static Type Struct(const TypeBuilder& typeBuilder, llvm::ArrayRef<StructMember> members);
			
			/**
			 * \brief Auto-aligned Struct Type
			 */
			static Type AutoStruct(const TypeBuilder& typeBuilder, llvm::ArrayRef<Type> memberTypes);
			
			/**
			 * \brief Array Type
			 */
			static Type Array(const TypeBuilder& typeBuilder, size_t elementCount, Type elementType);
			
			/**
			 * \brief Vector Type
			 */
			static Type Vector(const TypeBuilder& typeBuilder, size_t elementCount, Type elementType);
			
			Type() : kind_(VoidType) { }
			
			bool operator==(const Type& type) const;
			bool operator!=(const Type& type) const;
			bool operator<(const Type& type) const;
			
			TypeKind kind() const;
			
			bool isVoid() const;
			
			bool isPointer() const;
			
			bool isInteger() const;
			
			bool isUnspecifiedWidthInteger() const;
			
			IntegerKind integerKind() const;
			
			bool isFixedWidthInteger() const;
			
			DataSize integerWidth() const;
			bool integerIsSigned() const;
			
			bool isFloatingPoint() const;
			bool isFloat() const;
			bool isDouble() const;
			bool isLongDouble() const;
			
			FloatingPointKind floatingPointKind() const;
			
			bool isComplex() const;
			
			FloatingPointKind complexKind() const;
			
			bool isStruct() const;
			
			llvm::ArrayRef<StructMember> structMembers() const;
			
			bool isArray() const;
			
			size_t arrayElementCount() const;
			
			Type arrayElementType() const;
			
			bool isVector() const;
			
			size_t vectorElementCount() const;
			
			Type vectorElementType() const;
			
			bool hasUnalignedFields(const ABITypeInfo& typeInfo) const;
			
			bool hasSignedIntegerRepresentation(const ABITypeInfo& typeInfo) const;
			
			bool hasUnsignedIntegerRepresentation(const ABITypeInfo& typeInfo) const;
			
			size_t hash() const;
			
			std::string toString() const;
			
			struct TypeData;
			
		private:
			Type(TypeKind kind);
			
			TypeKind kind_;
			
			union {
				IntegerKind integerKind;
				struct {
					DataSize width;
					bool isSigned;
				} fixedWidthInteger;
				DataSize integerWidth;
				FloatingPointKind floatingPointKind;
				FloatingPointKind complexKind;
				
				// Aggregate types are uniqued by a pointer.
				const TypeData* uniquedPointer;
			} subKind_;
			
	};
	
	/**
	 * \brief ABI Struct Member
	 */
	class StructMember {
		public:
			static StructMember AutoOffset(const Type type) {
				return StructMember(type, DataSize::Bytes(0));
			}
			
			static StructMember ForceOffset(const Type type,
			                                const DataSize offset) {
				return StructMember(type, offset);
			}
			
			Type type() const {
				return type_;
			}
			
			DataSize offset() const {
				return offset_;
			}
			
			bool operator==(const StructMember& other) const {
				return type() == other.type() &&
				       offset() == other.offset();
			}
			
			bool operator<(const StructMember& other) const {
				if (type() != other.type()) {
					return type() < other.type();
				}
				
				if (offset() != other.offset()) {
					return offset() < other.offset();
				}
				
				return false;
			}
			
		private:
			StructMember(const Type pType,
			             const DataSize pOffset)
			: type_(pType), offset_(pOffset) { }
			
			Type type_;
			DataSize offset_;
			
	};
	
	struct Type::TypeData {
		struct {
			llvm::SmallVector<StructMember, 8> members;
		} structType;
		
		struct ArrayTypeData {
			size_t elementCount;
			Type elementType;
			
			ArrayTypeData()
			: elementCount(0) { }
		} arrayType;
		
		struct VectorTypeData {
			size_t elementCount;
			Type elementType;
			
			VectorTypeData()
			: elementCount(0) { }
		} vectorType;
		
		bool operator<(const TypeData& other) const {
			if (structType.members != other.structType.members) {
				return structType.members < other.structType.members;
			}
			
			if (arrayType.elementCount != other.arrayType.elementCount) {
				return arrayType.elementCount < other.arrayType.elementCount;
			}
			
			if (arrayType.elementType != other.arrayType.elementType) {
				return arrayType.elementType < other.arrayType.elementType;
			}
			
			if (vectorType.elementCount != other.vectorType.elementCount) {
				return vectorType.elementCount < other.vectorType.elementCount;
			}
			
			if (vectorType.elementType != other.vectorType.elementType) {
				return vectorType.elementType < other.vectorType.elementType;
			}
			
			return false;
		}
	};
	
	static const Type VoidTy = Type::Void();
	
	static const Type PointerTy = Type::Pointer();
	
	static const Type BoolTy = Type::UnspecifiedWidthInteger(Bool);
	static const Type CharTy = Type::UnspecifiedWidthInteger(Char);
	static const Type SCharTy = Type::UnspecifiedWidthInteger(SChar);
	static const Type UCharTy = Type::UnspecifiedWidthInteger(UChar);
	static const Type ShortTy = Type::UnspecifiedWidthInteger(Short);
	static const Type UShortTy = Type::UnspecifiedWidthInteger(UShort);
	static const Type IntTy = Type::UnspecifiedWidthInteger(Int);
	static const Type UIntTy = Type::UnspecifiedWidthInteger(UInt);
	static const Type LongTy = Type::UnspecifiedWidthInteger(Long);
	static const Type ULongTy = Type::UnspecifiedWidthInteger(ULong);
	static const Type LongLongTy = Type::UnspecifiedWidthInteger(LongLong);
	static const Type ULongLongTy = Type::UnspecifiedWidthInteger(ULongLong);
	
	static const Type IntPtrTy = Type::UnspecifiedWidthInteger(IntPtrT);
	static const Type UIntPtrTy = Type::UnspecifiedWidthInteger(UIntPtrT);
	static const Type PtrDiffTy = Type::UnspecifiedWidthInteger(PtrDiffT);
	static const Type SizeTy = Type::UnspecifiedWidthInteger(SizeT);
	static const Type SSizeTy = Type::UnspecifiedWidthInteger(SSizeT);
	
	static const Type Int8Ty = Type::FixedWidthInteger(DataSize::Bits(8), /*isSigned=*/true);
	static const Type UInt8Ty = Type::FixedWidthInteger(DataSize::Bits(8), /*isSigned=*/false);
	static const Type Int16Ty = Type::FixedWidthInteger(DataSize::Bits(16), /*isSigned=*/true);
	static const Type UInt16Ty = Type::FixedWidthInteger(DataSize::Bits(16), /*isSigned=*/false);
	static const Type Int24Ty = Type::FixedWidthInteger(DataSize::Bits(24), /*isSigned=*/true);
	static const Type UInt24Ty = Type::FixedWidthInteger(DataSize::Bits(24), /*isSigned=*/false);
	static const Type Int32Ty = Type::FixedWidthInteger(DataSize::Bits(32), /*isSigned=*/true);
	static const Type UInt32Ty = Type::FixedWidthInteger(DataSize::Bits(32), /*isSigned=*/false);
	static const Type Int64Ty = Type::FixedWidthInteger(DataSize::Bits(64), /*isSigned=*/true);
	static const Type UInt64Ty = Type::FixedWidthInteger(DataSize::Bits(64), /*isSigned=*/false);
	static const Type Int128Ty = Type::FixedWidthInteger(DataSize::Bits(128), /*isSigned=*/true);
	static const Type UInt128Ty = Type::FixedWidthInteger(DataSize::Bits(128), /*isSigned=*/false);
	
	static const Type FloatTy = Type::FloatingPoint(Float);
	static const Type DoubleTy = Type::FloatingPoint(Double);
	static const Type LongDoubleTy = Type::FloatingPoint(LongDouble);
	static const Type Float128Ty = Type::FloatingPoint(Float128);
	
}

namespace std {
	
	template <> struct hash<llvm_abi::Type> {
		size_t operator()(const llvm_abi::Type& type) const {
			return type.hash();
		}
	};
	
}

#endif
