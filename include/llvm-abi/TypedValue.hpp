#ifndef LLVMABI_TYPEDVALUE_HPP
#define LLVMABI_TYPEDVALUE_HPP

#include <utility>

#include <llvm/IR/Value.h>

namespace llvm_abi {
	
	class Type;
	
	using TypedValue = std::pair<llvm::Value*, Type>;

}

#endif
