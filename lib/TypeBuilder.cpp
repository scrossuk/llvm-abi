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
		return VoidTy;
	}
	
	Type TypeBuilder::getPointerTy() const {
		return PointerTy;
	}
	
	Type TypeBuilder::getBoolTy() const {
		return BoolTy;
	}
	
	Type TypeBuilder::getCharTy() const {
		return CharTy;
	}
	
	Type TypeBuilder::getShortTy() const {
		return ShortTy;
	}
	
	Type TypeBuilder::getIntTy() const {
		return IntTy;
	}
	
	Type TypeBuilder::getLongTy() const {
		return LongTy;
	}
	
	Type TypeBuilder::getLongLongTy() const {
		return LongLongTy;
	}
	
	Type TypeBuilder::getFloatTy() const {
		return FloatTy;
	}
	
	Type TypeBuilder::getDoubleTy() const {
		return DoubleTy;
	}
	
	Type TypeBuilder::getLongDoubleTy() const {
		return LongDoubleTy;
	}
	
	Type TypeBuilder::getFloat128Ty() const {
		return Float128Ty;
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

