#ifndef LLVMABI_ABI_HPP
#define LLVMABI_ABI_HPP

#include <memory>
#include <string>
#include <vector>

#include <llvm/ADT/Triple.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/Builder.hpp>
#include <llvm-abi/Type.hpp>

namespace llvm_abi {
	
	/**
	 * \brief ABI Interface
	 * 
	 * This interface represents an ABI for a particular
	 * target and provides access to information related
	 * to the ABI, such as type sizes/alignments. It also
	 * provides methods to encode/decode values when making
	 * function calls.
	 */
	class ABI {
	public:
		/**
		 * \brief Destructor.
		 */
		virtual ~ABI() { }
		
		/**
		 * \brief Get ABI name.
		 * 
		 * \return ABI name.
		 */
		virtual std::string name() const = 0;
		
		/**
		 * \brief Get the size of a type for this ABI.
		 * 
		 * \param type The type.
		 * \return The size of the type.
		 */
		virtual size_t typeSize(Type* type) const = 0;
		
		/**
		 * \brief Get the alignment of a type for this ABI.
		 * 
		 * \param type The type.
		 * \return The alignment of the type.
		 */
		virtual size_t typeAlign(Type* type) const = 0;
		
		/**
		 * \brief Create an array of offsets based on struct member types.
		 * 
		 * \param structMembers The member types of the struct.
		 * \return The offsets of each member of the struct.
		 */
		virtual std::vector<size_t> calculateStructOffsets(llvm::ArrayRef<StructMember> structMembers) const = 0;
		
		/**
		 * \brief Get the LLVM type used to represent a 'long double' for this ABI.
		 * 
		 * \return The LLVM type representing a 'long double'.
		 */
		virtual llvm::Type* longDoubleType() const = 0;
		
		/**
		 * \brief Encode an array of values with ABI types for an ABI-compliant function call.
		 * 
		 * This method is called to encode values when calling a function, as well as encoding values
		 * when returning from a function.
		 * 
		 * \param builder The builder for generating LLVM instructions.
		 * \param argValues An array of unencoded values; this is modified to create the encoded equivalents.
		 * \param argTypes An array of ABI types for each of the values.
		 */
		virtual void encodeValues(Builder& builder, std::vector<llvm::Value*>& argValues, llvm::ArrayRef<Type*> argTypes) = 0;
		
		/**
		 * \brief Decode an array of values with ABI types for an ABI-compliant function call.
		 * 
		 * This method is called to decode values when receiving function parameters at the
		 * beginning of a function, as well as decoding values returned from a function.
		 * 
		 * \param builder The builder for generating LLVM instructions.
		 * \param argValues An array of encoded values; this is modified to create the decoded equivalents.
		 * \param argTypes An array of ABI types for each of the values.
		 * \param llvmArgTypes An array of LLVM types for the decoded values.
		 */
		virtual void decodeValues(Builder& builder, std::vector<llvm::Value*>& argValues, llvm::ArrayRef<Type*> argTypes, llvm::ArrayRef<llvm::Type*> llvmArgTypes) = 0;
		
		/**
		 * \brief Re-write function type.
		 * 
		 * Modify a function type to use an ABI-compliant signature.
		 * 
		 * \param The LLVM function type.
		 * \param The ABI function type.
		 * \return The re-written LLVM function type.
		 */
		virtual llvm::FunctionType* rewriteFunctionType(llvm::FunctionType* llvmFunctionType, const FunctionType& functionType) = 0;
		
	};
	
	/**
	 * \brief Create an ABI for the specified target triple.
	 * 
	 * \param module The LLVM module.
	 * \param targetTriple the LLVM target triple.
	 * \return The ABI for the target.
	 */
	std::unique_ptr<ABI> createABI(llvm::Module* module, const llvm::Triple& targetTriple);

}

#endif
