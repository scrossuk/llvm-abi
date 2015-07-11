#ifndef LLVMABI_CALLINGCONVENTION_HPP
#define LLVMABI_CALLINGCONVENTION_HPP

#include <llvm/Support/ErrorHandling.h>

namespace llvm_abi {
	
	/**
	 * \brief Calling Convention
	 * 
	 * This enum specifies all possible ABI calling conventions.
	 */
	enum CallingConvention {
		/**
		 * \brief Default C Calling Convention
		 */
		CC_CDefault,
		
		/**
		 * \brief Default C++ Calling Convention
		 */
		CC_CppDefault,
		
		/**
		 * \brief 'cdecl' calling convention
		 */
		CC_CDecl,
		
		/**
		 * \brief 'stdcall' calling convention
		 */
		CC_StdCall,
		
		/**
		 * \brief 'fastcall' calling convention
		 */
		CC_FastCall,
		
		/**
		 * \brief 'thiscall' calling convention
		 */
		CC_ThisCall,
		
		/**
		 * \brief 'pascal' calling convention
		 */
		CC_Pascal,
		
		/**
		 * \brief MSVC calling convention that passes vectors and
		 *        vector aggregates in SSE registers.
		 */
		CC_VectorCall
	};
	
	inline const char*
	callingConventionString(const CallingConvention callingConvention) {
		switch (callingConvention) {
			case CC_CDefault:
				return "c-default";
			case CC_CppDefault:
				return "c++-default";
			case CC_CDecl:
				return "cdecl";
			case CC_StdCall:
				return "stdcall";
			case CC_FastCall:
				return "fastcall";
			case CC_ThisCall:
				return "thiscall";
			case CC_Pascal:
				return "pascal";
			case CC_VectorCall:
				return "vectorcall";
		}
		llvm_unreachable("Unknown calling convention.");
	}
	
}

#endif
