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

#include <llvm-abi/CallingConvention.hpp>

namespace llvm_abi {
	
	class Builder;
	class FunctionEncoder;
	class FunctionType;
	class StructMember;
	class Type;
	
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
		virtual size_t typeSize(Type type) const = 0;
		
		/**
		 * \brief Get the alignment of a type for this ABI.
		 * 
		 * \param type The type.
		 * \return The alignment of the type.
		 */
		virtual size_t typeAlign(Type type) const = 0;
		
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
		virtual std::vector<size_t> calculateStructOffsets(llvm::ArrayRef<StructMember> structMembers) const = 0;
		
		/**
		 * \brief Get the LLVM type used to represent a 'long double' for this ABI.
		 * 
		 * \return The LLVM type representing a 'long double'.
		 */
		virtual llvm::Type* longDoubleType() const = 0;
		
		/**
		 * \brief Get the LLVM calling convention for this ABI.
		 * 
		 * \param The generic ABI calling convention.
		 * \return The LLVM calling convention ID.
		 */
		virtual llvm::CallingConv::ID getCallingConvention(CallingConvention callingConvention) const = 0;
		
		/**
		 * \brief Get LLVM function type.
		 * 
		 * This creates an LLVM function type with an ABI-compliant signature.
		 * 
		 * \param The ABI function type.
		 * \return The LLVM function type.
		 */
		virtual llvm::FunctionType* getFunctionType(const FunctionType& functionType) const = 0;
		
		/**
		 * \brief Get function attributes for ABI.
		 * 
		 * This creates a set of attributes as needed by the ABI for a
		 * function of the given type. Existng attributes should be
		 * passed to this method since some may need to be disabled
		 * (e.g. 'readnone' disabled when arguments are passed via
		 * indirect pointers).
		 * 
		 * \param functionType The ABI function type.
		 * \param existingAttributes The existing function attributes.
		 * \return The set of attributes for the ABI.
		 */
		virtual llvm::AttributeSet getAttributes(const FunctionType& functionType,
		                                         llvm::AttributeSet existingAttributes = llvm::AttributeSet()) const = 0;
		
		/**
		 * \brief Create a function call.
		 * 
		 * Emits a function call, taking the given ABI-independent
		 * argument values and returning an ABI-independent value.
		 * 
		 * This method should be given a function that emits the desired
		 * call instruction, which provides flexibility (e.g. to use
		 * an invoke instruction rather than call). This function is
		 * given the ABI-encoded function arguments and should return the
		 * function call return value (itself also ABI-encoded).
		 * 
		 * \param builder The builder for emitting instructions.
		 * \param functionType The ABI function type.
		 * \param callBuilder A function that should emit the necessary call.
		 * \param argument The ABI-independent function arguments.
		 * \return The decoded function return value.
		 */
		virtual llvm::Value* createCall(Builder& builder,
		                                const FunctionType& functionType,
		                                std::function<llvm::Value* (llvm::ArrayRef<llvm::Value*>)> callBuilder,
		                                llvm::ArrayRef<llvm::Value*> arguments) = 0;
		
		/**
		 * \brief Create function.
		 * 
		 * Creates a 'function encoder' that can be used to obtain the
		 * ABI-independent arguments as well as return an ABI-independent
		 * value.
		 * 
		 * \brief builder The builder for emitting instructions.
		 * \param functionType The ABI function type.
		 * \param arguments The ABI-encoded function arguments.
		 * \return A function encoder instance.
		 */
		virtual std::unique_ptr<FunctionEncoder> createFunction(Builder& builder,
		                                                        const FunctionType& functionType,
		                                                        llvm::ArrayRef<llvm::Value*> arguments) = 0;
		
	};
	
	/**
	 * \brief Create an ABI for the specified target triple.
	 * 
	 * \param module The LLVM module.
	 * \param targetTriple the LLVM target triple.
	 * \return The ABI for the target.
	 */
	std::unique_ptr<ABI> createABI(llvm::Module& module,
	                               const llvm::Triple& targetTriple,
	                               const std::string& cpu = "");

}

#endif
