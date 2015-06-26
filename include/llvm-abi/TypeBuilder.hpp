#ifndef LLVMABI_TYPEBUILDER_HPP
#define LLVMABI_TYPEBUILDER_HPP

#include <set>

#include <llvm-abi/Type.hpp>

namespace llvm_abi {
	
	/**
	 * \brief ABI Type Builder
	 * 
	 * This class 'uniques' aggregate types so that types can passed around
	 * with an internal pointer which means that comparison simply involves
	 * comparing the pointers and copying is just copying the pointers. It
	 * also has convenience methods for primitive values (e.g. int).
	 */
	class TypeBuilder {
		public:
			TypeBuilder();
			
			const Type::TypeData* getUniquedTypeData(Type::TypeData typeData) const;
			
			Type getVoidTy() const;
			
			Type getPointerTy() const;
			
			Type getBoolTy() const;
			Type getCharTy() const;
			Type getShortTy() const;
			Type getIntTy() const;
			Type getLongTy() const;
			Type getLongLongTy() const;
			
			Type getFloatTy() const;
			Type getDoubleTy() const;
			Type getLongDoubleTy() const;
			Type getFloat128Ty() const;
			
			Type getStructTy(std::initializer_list<Type> memberTypes) const;
			Type getStructTy(llvm::ArrayRef<Type> memberTypes) const;
			
			Type getArrayTy(size_t elementCount, Type elementType) const;
			
			Type getVectorTy(size_t elementCount, Type elementType) const;
			
		private:
			// Non-copyable.
			TypeBuilder(const TypeBuilder&) = delete;
			TypeBuilder& operator=(const TypeBuilder&) = delete;
			
			mutable std::set<Type::TypeData> typeDataSet_;
			
	};
	
}

#endif
