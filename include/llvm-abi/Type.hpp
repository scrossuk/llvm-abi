#ifndef LLVMABI_ABITYPE_HPP
#define LLVMABI_ABITYPE_HPP

#include <string>
#include <vector>

#include <llvm/ADT/ArrayRef.h>

namespace llvm_abi {
	
	class Type;
	
	/**
	 * \brief ABI Struct Member
	 */
	class StructMember {
		public:
			static StructMember AutoOffset(const Type* type) {
				return StructMember(type, 0);
			}
			
			static StructMember ForceOffset(const Type* type, size_t offset) {
				return StructMember(type, offset);
			}
			
			const Type* type() const {
				return type_;
			}
			
			size_t offset() const {
				return offset_;
			}
			
		private:
			StructMember(const Type* pType, size_t pOffset)
				: type_(pType), offset_(pOffset) { }
			
			const Type* type_;
			size_t offset_;
			
	};
	
	/**
	 * \brief Integer Kind
	 * 
	 * A set of possible integer types.
	 */
	enum IntegerKind {
		Bool,
		Char,
		Short,
		Int,
		Long,
		LongLong,
		Int8,
		Int16,
		Int32,
		Int64,
		Int128,
		SizeT,
		PtrDiffT
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
		IntegerType,
		FloatingPointType,
		ComplexType,
		StructType,
		ArrayType
	};
	
	// Forward declaration.
	class Context;
	class StructMember;
	
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
			static const Type* Void(Context& context);
			
			/**
			 * \brief Integer Type
			 */
			static const Type* Pointer(Context& context);
			
			/**
			 * \brief Integer Type
			 */
			static const Type* Integer(Context& context, IntegerKind kind);
			
			/**
			 * \brief Floating Point Type
			 */
			static const Type* FloatingPoint(Context& context, FloatingPointKind kind);
			
			/**
			 * \brief Complex Type
			 */
			static const Type* Complex(Context& context, FloatingPointKind kind);
			
			/**
			 * \brief Struct Type
			 */
			static const Type* Struct(Context& context, llvm::ArrayRef<StructMember> members);
			
			/**
			 * \brief Auto-aligned Struct Type
			 */
			static const Type* AutoStruct(Context& context, llvm::ArrayRef<const Type*> memberTypes);
			
			/**
			 * \brief Array Type
			 */
			static const Type* Array(Context& context, size_t elementCount, const Type* elementType);
			
			bool operator<(const Type& type) const;
			
			TypeKind kind() const;
			
			bool isVoid() const;
			
			bool isPointer() const;
			
			bool isInteger() const;
			
			IntegerKind integerKind() const;
			
			bool isFloatingPoint() const;
			
			FloatingPointKind floatingPointKind() const;
			
			bool isComplex() const;
			
			FloatingPointKind complexKind() const;
			
			bool isStruct() const;
			
			llvm::ArrayRef<StructMember> structMembers() const;
			
			bool isArray() const;
			
			size_t arrayElementCount() const;
			
			const Type* arrayElementType() const;
			
			std::string toString() const;
			
		private:
			Type(TypeKind kind);
			
			TypeKind kind_;
			
			union {
				IntegerKind integerKind;
				FloatingPointKind floatingPointKind;
				FloatingPointKind complexKind;
			} subKind_;
			
			struct {
				llvm::SmallVector<StructMember, 8> members;
			} structType_;
			
			struct {
				size_t elementCount;
				const Type* elementType;
			} arrayType_;
			
	};
	
}

#endif
