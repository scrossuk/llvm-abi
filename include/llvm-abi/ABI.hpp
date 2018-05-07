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
#include <llvm-abi/TypedValue.hpp>

namespace llvm_abi {
	
	class ABITypeInfo;
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
		 * \brief Get ABI type information.
		 * 
		 * \return ABI type information.
		 */
		virtual const ABITypeInfo& typeInfo() const = 0;
		
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
		 * This creates a list of attributes as needed by the ABI for a
		 * function of the given type. Existng attributes should be
		 * passed to this method since some may need to be disabled
		 * (e.g. 'readnone' disabled when arguments are passed via
		 * indirect pointers).
		 * 
		 * \param functionType The ABI function type.
		 * \param existingAttributes The existing function attributes.
		 * \return The set of attributes for the ABI.
		 */
		virtual llvm::AttributeList getAttributes(const FunctionType& functionType,
		                                          llvm::ArrayRef<Type> argumentTypes,
		                                          llvm::AttributeList existingAttributes = llvm::AttributeList()) const = 0;
		
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
		                                llvm::ArrayRef<TypedValue> arguments) const = 0;
		
		/**
		 * \brief Create function encoder.
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
		virtual std::unique_ptr<FunctionEncoder> createFunctionEncoder(Builder& builder,
		                                                               const FunctionType& functionType,
		                                                               llvm::ArrayRef<llvm::Value*> arguments) const = 0;
		
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
