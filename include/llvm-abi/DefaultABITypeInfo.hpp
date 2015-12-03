#ifndef LLVMABI_DEFAULTABITYPEINFO_HPP
#define LLVMABI_DEFAULTABITYPEINFO_HPP

#include <map>
#include <string>

#include <llvm/IR/Type.h>

namespace llvm_abi {
	
	class DataSize;
	class RecordMember;
	class Type;
	
	class DefaultABITypeInfoDelegate {
	public:
		virtual DataSize getPointerSize() const = 0;
		virtual DataSize getPointerAlign() const = 0;
		
		virtual DataSize getIntSize(IntegerKind kind) const = 0;
		virtual DataSize getIntAlign(IntegerKind kind) const = 0;
		
		virtual DataSize getFloatSize(FloatingPointKind kind) const = 0;
		virtual DataSize getFloatAlign(FloatingPointKind kind) const = 0;
		
		virtual DataSize getComplexSize(FloatingPointKind kind) const = 0;
		virtual DataSize getComplexAlign(FloatingPointKind kind) const = 0;
		
		virtual DataSize getArrayAlign(Type type) const = 0;
		virtual DataSize getVectorAlign(Type type) const = 0;
		
		virtual llvm::Type* getLongDoubleIRType() const = 0;
		
	protected:
		// Prevent destructor call via this class.
		~DefaultABITypeInfoDelegate() { }
		
	};
	
	/**
	 * \brief Default ABI Type Information
	 * 
	 * This class contains ABI type information functionality that is
	 * typically common to all ABIs, such as how to layout structs.
	 */
	class DefaultABITypeInfo {
	public:
		DefaultABITypeInfo(llvm::LLVMContext& llvmContext,
		                   const ABITypeInfo& typeInfo,
		                   const DefaultABITypeInfoDelegate& delegate);
		~DefaultABITypeInfo();
		
		/**
		 * \brief Get the size of a type for this ABI.
		 * 
		 * \param type The ABI type.
		 * \return The size of the type.
		 */
		DataSize getDefaultTypeRawSize(Type type) const;
		
		/**
		 * \brief Get the allocation size of a type for this ABI.
		 * 
		 * \param type The ABI type.
		 * \return The size of the type.
		 */
		DataSize getDefaultTypeAllocSize(Type type) const;
		
		/**
		 * \brief Get the store size of a type for this ABI.
		 * 
		 * \param type The ABI type.
		 * \return The size of the type.
		 */
		DataSize getDefaultTypeStoreSize(Type type) const;
		
		/**
		 * \brief Get the required alignment of a type for this ABI.
		 * 
		 * \param type The ABI type.
		 * \return The alignment of the type.
		 */
		DataSize getDefaultTypeRequiredAlign(Type type) const;
		
		/**
		 * \brief Get the preferred alignment of a type for this ABI.
		 * 
		 * \param type The ABI type.
		 * \return The alignment of the type.
		 */
		DataSize getDefaultTypePreferredAlign(Type type) const;
		
		/**
		 * \brief Get LLVM struct type with name and member types.
		 * 
		 * \param name The name of the struct (empty string for for literal struct).
		 * \param members The struct member types.
		 * \return The LLVM struct type.
		 */
		llvm::StructType* getLLVMStructType(const std::string& name,
		                                    llvm::ArrayRef<llvm::Type*> members) const;
		
		/**
		 * \brief Get the LLVM type used to represent the ABI type given.
		 * 
		 * \param type The ABI type.
		 * \return The LLVM type representing the ABI type.
		 */
		llvm::Type* getDefaultLLVMType(Type type) const;
		
		/**
		 * \brief Create an array of offsets based on struct member types.
		 * 
		 * \param structMembers The member types of the struct.
		 * \return The offsets of each member of the struct.
		 */
		llvm::SmallVector<DataSize, 8>
		calculateDefaultStructOffsets(llvm::ArrayRef<RecordMember> structMembers) const;
		
	private:
		llvm::LLVMContext& llvmContext_;
		const ABITypeInfo& typeInfo_;
		const DefaultABITypeInfoDelegate& delegate_;
		mutable std::map<std::string, llvm::StructType*> structTypes_;
		
	};

}

#endif
