#ifndef LLVMABI_SUPPORT_HPP
#define LLVMABI_SUPPORT_HPP

#include <cassert>
#include <cstddef>

namespace llvm_abi {
	
	inline bool isPowerOf2(size_t value) {
		return value != 0 && (value & (value - 1)) == 0;
	}
	
	inline size_t roundUpToAlign(size_t position, size_t align) {
		assert(isPowerOf2(align));
		return (position + (align - 1)) & (~(align - 1));
	}
	
}

#endif