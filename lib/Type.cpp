#include <assert.h>

#include <vector>

#include <llvm/Support/ErrorHandling.h>

#include <llvm-abi/Context.hpp>
#include <llvm-abi/Type.hpp>

namespace llvm_abi {
	
	const Type* Type::Void(Context& context) {
		Type type(VoidType);
		return context.getType(type);
	}
	
	const Type* Type::Pointer(Context& context) {
		Type type(PointerType);
		return context.getType(type);
	}
	
	const Type* Type::Integer(Context& context, IntegerKind kind) {
		Type type(IntegerType);
		type.subKind_.integerKind = kind;
		return context.getType(type);
	}
	
	const Type* Type::FloatingPoint(Context& context, FloatingPointKind kind) {
		Type type(FloatingPointType);
		type.subKind_.floatingPointKind = kind;
		return context.getType(type);
	}
	
	const Type* Type::Complex(Context& context, FloatingPointKind kind) {
		Type type(ComplexType);
		type.subKind_.complexKind = kind;
		return context.getType(type);
	}
	
	const Type* Type::Struct(Context& context, llvm::ArrayRef<StructMember> members) {
		Type type(StructType);
		type.structType_.members = llvm::SmallVector<StructMember, 8>(members.begin(), members.end());
		return context.getType(type);
	}
	
	const Type* Type::AutoStruct(Context& context, llvm::ArrayRef<const Type*> memberTypes) {
		Type type(StructType);
		type.structType_.members.reserve(memberTypes.size());
		for (auto& memberType: memberTypes) {
			type.structType_.members.push_back(StructMember::AutoOffset(memberType));
		}
		return context.getType(type);
	}
	
	const Type* Type::Array(Context& context, size_t elementCount, const Type* elementType) {
		Type type(ArrayType);
		type.arrayType_.elementCount = elementCount;
		type.arrayType_.elementType = elementType;
		return context.getType(type);
	}
	
	Type::Type(TypeKind pKind)
		: kind_(pKind) { }
	
	bool Type::operator<(const Type& type) const {
		if (kind() != type.kind()) {
			return kind() < type.kind();
		}
		
		switch (kind()) {
			case PointerType:
				return false;
			case IntegerType:
				return integerKind() < type.integerKind();
			case FloatingPointType:
				return floatingPointKind() < type.floatingPointKind();
			case ComplexType:
				return complexKind() < type.complexKind();
			case StructType: {
				if (structMembers().size() != type.structMembers().size()) {
					return structMembers().size() < type.structMembers().size();
				}
				
				for (size_t i = 0; i < structMembers().size(); i++) {
					const auto& myMember = structMembers()[i];
					const auto& otherMember = type.structMembers()[i];
					
					if (myMember.type() != otherMember.type()) {
						return myMember.type() < otherMember.type();
					}
					
					if (myMember.offset() != otherMember.offset()) {
						return myMember.offset() < otherMember.offset();
					}
				}
				
				return false;
			}
			case ArrayType:
				if (arrayElementCount() != type.arrayElementCount()) {
					return arrayElementCount() < type.arrayElementCount();
				}
				
				return arrayElementType() < type.arrayElementType();
			default:
				llvm_unreachable("Unknown ABI Type kind in operator<().");
		}
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
		return structType_.members;
	}
	
	bool Type::isArray() const {
		return kind() == ArrayType;
	}
	
	size_t Type::arrayElementCount() const {
		assert(isArray());
		return arrayType_.elementCount;
	}
	
	const Type* Type::arrayElementType() const {
		assert(isArray());
		return arrayType_.elementType;
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
					s += std::string("StructMember(") + members[i].type()->toString() + ")";
				}
				return s + ")";
			}
			case ArrayType:
				return std::string("Array(") + arrayElementType()->toString() + ")";
		}
		
		llvm_unreachable("Unknown type kind.");
	}
	
}

