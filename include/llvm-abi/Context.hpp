#ifndef LLVMABI_CONTEXT_HPP
#define LLVMABI_CONTEXT_HPP

#include <set>

#include <llvm-abi/Type.hpp>

namespace llvm_abi {
	
	/**
	 * \brief ABI Context
	 * 
	 * This class 'uniques' all the types so that types
	 * are passed around by pointer, which means that
	 * comparison simply involves comparing the pointers
	 * and copying is just copying the pointers.
	 */
	class Context {
		public:
			Context();
			
			const Type* getType(Type type);
			
		private:
			// Non-copyable.
			Context(const Context&) = delete;
			Context& operator=(const Context&) = delete;
			
			std::set<Type> types_;
			
	};
	
}

#endif
