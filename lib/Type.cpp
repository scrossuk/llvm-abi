#include <assert.h>

#include <functional>
#include <sstream>
#include <vector>

#include <llvm/Support/ErrorHandling.h>

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Support.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

namespace llvm_abi {
	
	Type Type::Void() {
		return Type(VoidType);
	}
	
	Type Type::Pointer() {
		return Type(PointerType);
	}
	
	Type Type::Integer(IntegerKind kind) {
		Type type(IntegerType);
		type.subKind_.integerKind = kind;
		return type;
	}
	
	Type Type::getFixedSizeInt(const size_t intSizeInBytes) {
		switch (intSizeInBytes) {
			case 1:
				return Int8Ty;
			case 2:
				return Int16Ty;
			case 3:
				return Int24Ty;
			case 4:
				return Int32Ty;
			case 8:
				return Int64Ty;
			case 16:
				return Int128Ty;
			default:
				printf("%llu\n", (unsigned long long) intSizeInBytes);
				llvm_unreachable("Invalid fixed int size.");
		}
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
			case IntegerType:
				return integerKind() == type.integerKind();
			case FloatingPointType:
				return floatingPointKind() == type.floatingPointKind();
			case ComplexType:
				return complexKind() == type.complexKind();
			case StructType:
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
			case IntegerType:
				return integerKind() < type.integerKind();
			case FloatingPointType:
				return floatingPointKind() < type.floatingPointKind();
			case ComplexType:
				return complexKind() < type.complexKind();
			case StructType:
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
		return kind() == IntegerType;
	}
	
	IntegerKind Type::integerKind() const {
		assert(isInteger());
		return subKind_.integerKind;
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
		
		size_t offset = 0;
		
		for (const auto& member: structMembers()) {
			// Add necessary padding before this member.
			offset = roundUpToAlign(offset, typeInfo.getTypeAlign(member.type()));
			
			const auto memberOffset = member.offset() == 0 ? offset : member.offset();
			
			if (memberOffset != offset ||
			    member.type().hasUnalignedFields(typeInfo)) {
				return true;
			}
			
			// Add the member's size.
			offset += typeInfo.getTypeSize(member.type());
		}
		
		return false;
	}
	
	size_t Type::hash() const {
		// TODO: improve this!
		const size_t value = std::hash<unsigned long long>()(kind_);
		
		switch (kind()) {
			case VoidType:
			case PointerType:
				return value;
			case IntegerType:
				return value ^ std::hash<unsigned long long>()(integerKind());
			case FloatingPointType:
				return value ^ std::hash<unsigned long long>()(floatingPointKind());
			case ComplexType:
				return value ^ std::hash<unsigned long long>()(complexKind());
			case StructType:
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
			case Short:
				return "Short";
			case Int:
				return "Int";
			case Long:
				return "Long";
			case LongLong:
				return "LongLong";
			case Int8:
				return "Int8";
			case Int16:
				return "Int16";
			case Int24:
				return "Int24";
			case Int32:
				return "Int32";
			case Int64:
				return "Int64";
			case Int128:
				return "Int128";
			case SizeT:
				return "SizeT";
			case PtrDiffT:
				return "PtrDiffT";
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
			case IntegerType:
				return std::string("Integer(") + intKindToString(integerKind()) + ")";
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

