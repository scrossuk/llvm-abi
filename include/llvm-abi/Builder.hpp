#ifndef LLVMABI_BUILDER_HPP
#define LLVMABI_BUILDER_HPP

#include <llvm/IR/IRBuilder.h>

namespace llvm_abi {
	
	/**
	 * \brief LLVM IRBuilder.
	 */
	typedef llvm::IRBuilder<> IRBuilder;
	
	/**
	 * \brief Builder Interface
	 * 
	 * This is an interface provided by the client to the ABI
	 * to indicate where instructions should be emitted.
	 */
	class Builder {
	public:
		/**
		 * \brief Get an IRBuilder for the first basic block in the function.
		 * 
		 * This is used for generating allocas.
		 * 
		 * \return IRBuilder for the current function's first basic block.
		 */
		virtual IRBuilder& getEntryBuilder() = 0;
		
		/**
		 * \brief Get an IRBuilder for the current position in the function.
		 * 
		 * \return IRBuilder for the current function's code emitting position.
		 */
		virtual IRBuilder& getBuilder() = 0;
		
	protected:
		// Prevent destructor calls via this class.
		~Builder() { }
		
	};
	
}

#endif
