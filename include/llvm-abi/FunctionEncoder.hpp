#ifndef LLVMABI_FUNCTIONENCODER_HPP
#define LLVMABI_FUNCTIONENCODER_HPP

#include <llvm/IR/Value.h>

namespace llvm_abi {
	
	/**
	 * \brief Function Encoder
	 * 
	 * This class represents a function as it is being generated; instances
	 * of this can be created from an implementation of the 'ABI' interface.
	 */
	class FunctionEncoder {
	public:
		/**
		 * \brief Destructor.
		 */
		virtual ~FunctionEncoder() { }
		
		/**
		 * \brief Get function arguments.
		 * 
		 * This returns the arguments in ABI-independent form.
		 * 
		 * \return Decoded function arguments.
		 */
		virtual llvm::ArrayRef<llvm::Value*> arguments() const = 0;
		
		/**
		 * \brief Return a value.
		 * 
		 * Emits code to return the given value as an encoded return
		 * value.
		 * 
		 * \param Return value.
		 * \return The return instruction emitted.
		 */
		virtual llvm::ReturnInst* returnValue(llvm::Value* value) = 0;
		
	};
	
}

#endif
