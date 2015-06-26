#include <set>

#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

namespace llvm_abi {
	
	TypeBuilder::TypeBuilder() { }
	
	const Type::TypeData* TypeBuilder::getUniquedTypeData(Type::TypeData typeData) const {
		auto result = typeDataSet_.insert(std::move(typeData));
		return &(*(result.first));
	}
	
	Type TypeBuilder::getVoidTy() const {
		return Type::Void();
	}
	
	Type TypeBuilder::getPointerTy() const {
		return Type::Pointer();
	}
	
	Type TypeBuilder::getBoolTy() const {
		return Type::Integer(Bool);
	}
	
	Type TypeBuilder::getCharTy() const {
		return Type::Integer(Char);
	}
	
	Type TypeBuilder::getShortTy() const {
		return Type::Integer(Short);
	}
	
	Type TypeBuilder::getIntTy() const {
		return Type::Integer(Int);
	}
	
	Type TypeBuilder::getLongTy() const {
		return Type::Integer(Long);
	}
	
	Type TypeBuilder::getLongLongTy() const {
		return Type::Integer(LongLong);
	}
	
	Type TypeBuilder::getFloatTy() const {
		return Type::FloatingPoint(Float);
	}
	
	Type TypeBuilder::getDoubleTy() const {
		return Type::FloatingPoint(Double);
	}
	
	Type TypeBuilder::getLongDoubleTy() const {
		return Type::FloatingPoint(LongDouble);
	}
	
	Type TypeBuilder::getFloat128Ty() const {
		return Type::FloatingPoint(Float128);
	}
	
	Type TypeBuilder::getStructTy(std::initializer_list<Type> memberTypes) const {
		return Type::AutoStruct(*this,
		                        llvm::ArrayRef<Type>(memberTypes.begin(),
					                     memberTypes.end()));
	}
	
	Type TypeBuilder::getStructTy(llvm::ArrayRef<Type> memberTypes) const {
		return Type::AutoStruct(*this, memberTypes);
	}
	
	Type TypeBuilder::getArrayTy(const size_t elementCount,
	                             const Type elementType) const {
		return Type::Array(*this, elementCount, elementType);
	}
	
	Type TypeBuilder::getVectorTy(const size_t elementCount,
	                              const Type elementType) const {
		return Type::Vector(*this, elementCount, elementType);
	}
	
}

