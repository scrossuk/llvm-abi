#ifndef LLVMABI_ARGUMENTIRMAPPING_HPP
#define LLVMABI_ARGUMENTIRMAPPING_HPP

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/ArgInfo.hpp>
#include <llvm-abi/FunctionType.hpp>

namespace llvm_abi {
	
	static const size_t InvalidIndex = ~0U;
	
	/**
	 * \brief Mapping from ABI argument to IR arguments.
	 * 
	 * This struct holds the information necessary to translate
	 * from an ABI argument to an LLVM IR argument. Specifically,
	 * each ABI argument has:
	 * 
	 *     * An ArgInfo value.
	 *     * A range of IR arguments.
	 *     * A padding IR argument.
	 * 
	 * The ABI argument is hence expanded to the IR arguments in
	 * the range [firstArgIndex, firstArgIndex + numberOfIRArgs).
	 */
	struct ArgumentIRMapping {
		size_t paddingArgIndex;
		size_t firstArgIndex;
		size_t numberOfIRArgs;
		
		ArgInfo argInfo;
		
		ArgumentIRMapping()
		: paddingArgIndex(InvalidIndex),
		firstArgIndex(InvalidIndex),
		numberOfIRArgs(0) {}
	};
	
}

#endif
