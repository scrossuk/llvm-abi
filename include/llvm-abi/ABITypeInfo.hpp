#ifndef LLVMABI_ABITYPEINFO_HPP
#define LLVMABI_ABITYPEINFO_HPP

#include <llvm/IR/Type.h>

// FIXME: Remove!
#include <llvm-abi/Type.hpp>

namespace llvm_abi {
	
	class DataSize;
	class StructMember;
	class Type;
	class TypeBuilder;
	
	/**
	 * \brief ABI Type Information
	 */
	class ABITypeInfo {
	public:
		/**
		 * \brief Get the type builder being used by this ABI.
		 * 
		 * \return The type builder being used to create ABI types.
		 */
		virtual const TypeBuilder& typeBuilder() const = 0;
		
		/**
		 * \brief Get the size of a type for this ABI.
		 * 
		 * \param type The ABI type.
		 * \return The size of the type.
		 */
		virtual DataSize getTypeRawSize(Type type) const = 0;
		
		/**
		 * \brief Get the allocation size of a type for this ABI.
		 * 
		 * \param type The ABI type.
		 * \return The size of the type.
		 */
		virtual DataSize getTypeAllocSize(Type type) const = 0;
		
		/**
		 * \brief Get the store size of a type for this ABI.
		 * 
		 * \param type The ABI type.
		 * \return The size of the type.
		 */
		virtual DataSize getTypeStoreSize(Type type) const = 0;
		
		/**
		 * \brief Get the required alignment of a type for this ABI.
		 * 
		 * \param type The ABI type.
		 * \return The alignment of the type.
		 */
		virtual DataSize getTypeRequiredAlign(Type type) const = 0;
		
		/**
		 * \brief Get the preferred alignment of a type for this ABI.
		 * 
		 * \param type The ABI type.
		 * \return The alignment of the type.
		 */
		virtual DataSize getTypePreferredAlign(Type type) const = 0;
		
		/**
		 * \brief Get the LLVM type used to represent the ABI type given.
		 * 
		 * \param type The ABI type.
		 * \return The LLVM type representing the ABI type.
		 */
		virtual llvm::Type* getLLVMType(Type type) const = 0;
		
		/**
		 * \brief Create an array of offsets based on struct member types.
		 * 
		 * \param structMembers The member types of the struct.
		 * \return The offsets of each member of the struct.
		 */
		virtual llvm::SmallVector<DataSize, 8> calculateStructOffsets(llvm::ArrayRef<StructMember> structMembers) const = 0;
		
		/**
		 * \brief Queries whether vector type is legal for target.
		 * 
		 * \param type The ABI vector type.
		 * \return Whether the vector type is legal.
		 */
		virtual bool isLegalVectorType(Type type) const = 0;
		
		/**
		 * \brief Queries whether ABI is big-endian.
		 * 
		 * \return Whether ABI is big-endian.
		 */
		virtual bool isBigEndian() const = 0;
		
		/**
		 * \brief Queries whether 'char' is signed for this ABI.
		 * 
		 * \return Whether 'char' is signed.
		 */
		virtual bool isCharSigned() const = 0;
		
		// FIXME!
		virtual bool isHomogeneousAggregateBaseType(Type) const { return false; }
		virtual bool isHomogeneousAggregateSmallEnough(Type, uint64_t) const { return false; }
		
	protected:
		// Prevent destructor call via this class.
		~ABITypeInfo() { }
		
	};

}

#endif
