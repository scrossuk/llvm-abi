#include <assert.h>

#include <functional>
#include <sstream>
#include <vector>

#include <llvm/Support/ErrorHandling.h>

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

namespace llvm_abi {
	
	Type Type::Void() {
		return Type(VoidType);
	}
	
	Type Type::Pointer() {
		return Type(PointerType);
	}
	
	Type Type::UnspecifiedWidthInteger(IntegerKind kind) {
		Type type(UnspecifiedWidthIntegerType);
		type.subKind_.integerKind = kind;
		return type;
	}
	
	Type Type::FixedWidthInteger(const DataSize width, const bool isSigned) {
		Type type(FixedWidthIntegerType);
		type.subKind_.fixedWidthInteger.width = width;
		type.subKind_.fixedWidthInteger.isSigned = isSigned;
		return type;
	}
	
	Type Type::FloatingPoint(FloatingPointKind kind) {
		Type type(FloatingPointType);
		type.subKind_.floatingPointKind = kind;
		return type;
	}
	
	Type Type::Complex(FloatingPointKind kind) {
		Type type(ComplexType);
		type.subKind_.complexKind = kind;
		return type;
	}
	
	Type Type::Struct(const TypeBuilder& typeBuilder, llvm::ArrayRef<StructMember> members) {
		TypeData typeData;
		typeData.structType.members = llvm::SmallVector<StructMember, 8>(members.begin(), members.end());
		
		const auto typeDataPtr = typeBuilder.getUniquedTypeData(std::move(typeData));
		
		Type type(StructType);
		type.subKind_.uniquedPointer = typeDataPtr;
		return type;
	}
	
	Type Type::AutoStruct(const TypeBuilder& typeBuilder, llvm::ArrayRef<Type> memberTypes) {
		TypeData typeData;
		typeData.structType.members.reserve(memberTypes.size());
		for (auto& memberType: memberTypes) {
			typeData.structType.members.push_back(StructMember::AutoOffset(memberType));
		}
		
		const auto typeDataPtr = typeBuilder.getUniquedTypeData(std::move(typeData));
		
		Type type(StructType);
		type.subKind_.uniquedPointer = typeDataPtr;
		return type;
	}
	
	Type Type::Union(const TypeBuilder& typeBuilder, llvm::ArrayRef<Type> memberTypes) {
		TypeData typeData;
		typeData.unionType.members = llvm::SmallVector<Type, 8>(memberTypes.begin(), memberTypes.end());
		
		const auto typeDataPtr = typeBuilder.getUniquedTypeData(std::move(typeData));
		
		Type type(UnionType);
		type.subKind_.uniquedPointer = typeDataPtr;
		return type;
	}
	
	Type Type::Array(const TypeBuilder& typeBuilder, size_t elementCount, Type elementType) {
		TypeData typeData;
		typeData.arrayType.elementCount = elementCount;
		typeData.arrayType.elementType = elementType;
		
		const auto typeDataPtr = typeBuilder.getUniquedTypeData(std::move(typeData));
		
		Type type(ArrayType);
		type.subKind_.uniquedPointer = typeDataPtr;
		return type;
	}
	
	Type Type::Vector(const TypeBuilder& typeBuilder, size_t elementCount, Type elementType) {
		TypeData typeData;
		typeData.vectorType.elementCount = elementCount;
		typeData.vectorType.elementType = elementType;
		
		const auto typeDataPtr = typeBuilder.getUniquedTypeData(std::move(typeData));
		
		Type type(VectorType);
		type.subKind_.uniquedPointer = typeDataPtr;
		return type;
	}
	
	Type::Type(TypeKind pKind)
		: kind_(pKind) { }
	
	bool Type::operator==(const Type& type) const {
		if (kind() != type.kind()) {
			return false;
		}
		
		switch (kind()) {
			case VoidType:
			case PointerType:
				return true;
			case UnspecifiedWidthIntegerType:
				return integerKind() == type.integerKind();
			case FixedWidthIntegerType:
				return integerWidth() == type.integerWidth() &&
				       integerIsSigned() == type.integerIsSigned();
			case FloatingPointType:
				return floatingPointKind() == type.floatingPointKind();
			case ComplexType:
				return complexKind() == type.complexKind();
			case StructType:
			case UnionType:
			case ArrayType:
			case VectorType: {
				return subKind_.uniquedPointer == type.subKind_.uniquedPointer;
			}
		}
		
		llvm_unreachable("Unknown ABI Type kind in operator==().");
	}
	
	bool Type::operator!=(const Type& type) const {
		return !(*this == type);
	}
	
	bool Type::operator<(const Type& type) const {
		if (kind() != type.kind()) {
			return kind() < type.kind();
		}
		
		switch (kind()) {
			case VoidType:
			case PointerType:
				return false;
			case UnspecifiedWidthIntegerType:
				return integerKind() < type.integerKind();
			case FixedWidthIntegerType: {
				if (integerWidth() != type.integerWidth()) {
					return integerWidth() < type.integerWidth();
				} else {
					return integerIsSigned() < type.integerIsSigned();
				}
			}
			case FloatingPointType:
				return floatingPointKind() < type.floatingPointKind();
			case ComplexType:
				return complexKind() < type.complexKind();
			case StructType:
			case UnionType:
			case ArrayType:
			case VectorType: {
				return subKind_.uniquedPointer < type.subKind_.uniquedPointer;
			}
		}
		
		llvm_unreachable("Unknown ABI Type kind in operator<().");
	}
	
	TypeKind Type::kind() const {
		return kind_;
	}
	
	bool Type::isVoid() const {
		return kind() == VoidType;
	}
	
	bool Type::isPointer() const {
		return kind() == PointerType;
	}
	
	bool Type::isInteger() const {
		return isUnspecifiedWidthInteger() ||
		       isFixedWidthInteger();
	}
	
	bool Type::isUnspecifiedWidthInteger() const {
		return kind() == UnspecifiedWidthIntegerType;
	}
	
	IntegerKind Type::integerKind() const {
		assert(isUnspecifiedWidthInteger());
		return subKind_.integerKind;
	}
	
	bool Type::isFixedWidthInteger() const {
		return kind() == FixedWidthIntegerType;
	}
	
	DataSize Type::integerWidth() const {
		assert(isFixedWidthInteger());
		return subKind_.fixedWidthInteger.width;
	}
	
	bool Type::integerIsSigned() const {
		assert(isFixedWidthInteger());
		return subKind_.fixedWidthInteger.isSigned;
	}
	
	bool Type::isFloatingPoint() const {
		return kind() == FloatingPointType;
	}
	
	bool Type::isFloat() const {
		return *this == FloatTy;
	}
	
	bool Type::isDouble() const {
		return *this == DoubleTy;
	}
	
	bool Type::isLongDouble() const {
		return *this == LongDoubleTy;
	}
	
	FloatingPointKind Type::floatingPointKind() const {
		assert(isFloatingPoint());
		return subKind_.floatingPointKind;
	}
	
	bool Type::isComplex() const {
		return kind() == ComplexType;
	}
	
	FloatingPointKind Type::complexKind() const {
		assert(isComplex());
		return subKind_.complexKind;
	}
	
	bool Type::isStruct() const {
		return kind() == StructType;
	}
	
	llvm::ArrayRef<StructMember> Type::structMembers() const {
		assert(isStruct());
		return subKind_.uniquedPointer->structType.members;
	}
	
	bool Type::isUnion() const {
		return kind() == UnionType;
	}
	
	llvm::ArrayRef<Type> Type::unionMembers() const {
		assert(isUnion());
		return subKind_.uniquedPointer->unionType.members;
	}
	
	bool Type::isArray() const {
		return kind() == ArrayType;
	}
	
	size_t Type::arrayElementCount() const {
		assert(isArray());
		return subKind_.uniquedPointer->arrayType.elementCount;
	}
	
	Type Type::arrayElementType() const {
		assert(isArray());
		return subKind_.uniquedPointer->arrayType.elementType;
	}
	
	bool Type::isVector() const {
		return kind() == VectorType;
	}
	
	size_t Type::vectorElementCount() const {
		assert(isVector());
		return subKind_.uniquedPointer->vectorType.elementCount;
	}
	
	Type Type::vectorElementType() const {
		assert(isVector());
		return subKind_.uniquedPointer->vectorType.elementType;
	}
	
	bool Type::hasUnalignedFields(const ABITypeInfo& typeInfo) const {
		if (!isStruct()) {
			return false;
		}
		
		auto offset = DataSize::Bytes(0);
		
		for (const auto& member: structMembers()) {
			// Add necessary padding before this member.
			offset = offset.roundUpToAlign(typeInfo.getTypeRequiredAlign(member.type()));
			
			const auto memberOffset = member.offset().asBits() == 0 ? offset : member.offset();
			
			if (memberOffset != offset ||
			    member.type().hasUnalignedFields(typeInfo)) {
				return true;
			}
			
			// Add the member's size.
			offset += typeInfo.getTypeAllocSize(member.type());
		}
		
		return false;
	}
	
	bool Type::hasSignedIntegerRepresentation(const ABITypeInfo& typeInfo) const {
		switch (kind()) {
			case VoidType:
			case PointerType:
			case FloatingPointType:
			case ComplexType:
			case StructType:
			case UnionType:
			case ArrayType:
				return false;
			case UnspecifiedWidthIntegerType:
				switch (integerKind()) {
					case Bool:
						return false;
					case Char:
						return typeInfo.isCharSigned();
					case SChar:
					case Short:
					case Int:
					case Long:
					case LongLong:
					case SSizeT:
					case IntPtrT:
						return true;
					case UChar:
					case UShort:
					case UInt:
					case ULong:
					case ULongLong:
					case SizeT:
					case PtrDiffT:
					case UIntPtrT:
						return false;
				}
			case FixedWidthIntegerType:
				return integerIsSigned();
			case VectorType: {
				llvm_unreachable("TODO");
			}
		}
		
		llvm_unreachable("Unknown ABI Type kind in hasSignedIntegerRepresentation().");
	}
			
	bool Type::hasUnsignedIntegerRepresentation(const ABITypeInfo& typeInfo) const {
		switch (kind()) {
			case VoidType:
			case PointerType:
			case FloatingPointType:
			case ComplexType:
			case StructType:
			case UnionType:
			case ArrayType:
				return false;
			case UnspecifiedWidthIntegerType:
				switch (integerKind()) {
					case Bool:
						return false;
					case Char:
						return !typeInfo.isCharSigned();
					case SChar:
					case Short:
					case Int:
					case Long:
					case LongLong:
					case SSizeT:
					case IntPtrT:
						return false;
					case UChar:
					case UShort:
					case UInt:
					case ULong:
					case ULongLong:
					case SizeT:
					case PtrDiffT:
					case UIntPtrT:
						return true;
				}
			case FixedWidthIntegerType:
				return !integerIsSigned();
			case VectorType: {
				llvm_unreachable("TODO");
			}
		}
		
		llvm_unreachable("Unknown ABI Type kind in hasUnsignedIntegerRepresentation().");
	}
	
	size_t Type::hash() const {
		// TODO: improve this!
		const size_t value = std::hash<unsigned long long>()(kind_);
		
		switch (kind()) {
			case VoidType:
			case PointerType:
				return value;
			case UnspecifiedWidthIntegerType:
				return value ^ std::hash<unsigned long long>()(integerKind());
			case FixedWidthIntegerType:
				return (value ^ std::hash<unsigned long long>()(integerWidth().asBits()) ^
				        std::hash<bool>()(integerIsSigned()));
			case FloatingPointType:
				return value ^ std::hash<unsigned long long>()(floatingPointKind());
			case ComplexType:
				return value ^ std::hash<unsigned long long>()(complexKind());
			case StructType:
			case UnionType:
			case ArrayType:
			case VectorType: {
				return value ^ std::hash<const TypeData*>()(subKind_.uniquedPointer);
			}
		}
		
		llvm_unreachable("Unknown ABI Type kind in hash().");
	}
	
	static std::string intKindToString(IntegerKind kind) {
		switch (kind) {
			case Bool:
				return "Bool";
			case Char:
				return "Char";
			case SChar:
				return "SChar";
			case UChar:
				return "UChar";
			case Short:
				return "Short";
			case UShort:
				return "UShort";
			case Int:
				return "Int";
			case UInt:
				return "UInt";
			case Long:
				return "Long";
			case ULong:
				return "ULong";
			case LongLong:
				return "LongLong";
			case ULongLong:
				return "ULongLong";
			case SizeT:
				return "SizeT";
			case SSizeT:
				return "SSizeT";
			case PtrDiffT:
				return "PtrDiffT";
			case IntPtrT:
				return "IntPtrT";
			case UIntPtrT:
				return "UIntPtrT";
		}
		
		llvm_unreachable("Unknown integer type kind.");
	}
	
	static std::string floatKindToString(FloatingPointKind kind) {
		switch (kind) {
			case Float:
				return "Float";
			case Double:
				return "Double";
			case LongDouble:
				return "LongDouble";
			case Float128:
				return "Float128";
		}
		
		llvm_unreachable("Unknown float type kind.");
	}
	
	std::string Type::toString() const {
		switch (kind()) {
			case VoidType:
				return "Void";
			case PointerType:
				return "Pointer";
			case UnspecifiedWidthIntegerType:
				return std::string("UnspecifiedWidthInteger(") + intKindToString(integerKind()) + ")";
			case FixedWidthIntegerType: {
				std::ostringstream stream;
				stream << "FixedWidthInteger(" << integerWidth().asBits() << " bits, ";
				stream << (integerIsSigned() ? "signed" : "unsigned") << ")";
				return stream.str();
			}
			case FloatingPointType:
				return std::string("FloatingPoint(") + floatKindToString(floatingPointKind()) + ")";
			case ComplexType:
				return std::string("Complex(") + floatKindToString(complexKind()) + ")";
			case StructType: {
				std::string s = "Struct(";
				const auto& members = structMembers();
				for (size_t i = 0; i < members.size(); i++) {
					if (i > 0) {
						s += ", ";
					}
					s += std::string("StructMember(") + members[i].type().toString() + ")";
				}
				return s + ")";
			}
			case UnionType: {
				std::string s = "Union(";
				const auto& members = unionMembers();
				for (size_t i = 0; i < members.size(); i++) {
					if (i > 0) {
						s += ", ";
					}
					s += members[i].toString();
				}
				return s + ")";
			}
			case ArrayType: {
				std::ostringstream stream;
				stream << "Array(" << arrayElementCount() << ", " << arrayElementType().toString() << ")";
				return stream.str();
			}
			case VectorType: {
				std::ostringstream stream;
				stream << "Vector(" << vectorElementCount() << ", " << vectorElementType().toString() << ")";
				return stream.str();
			}
		}
		
		llvm_unreachable("Unknown ABI Type kind in toString().");
	}
	
}

