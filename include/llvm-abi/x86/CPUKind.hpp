#ifndef LLVMABI_X86_64_CPUKIND_HPP
#define LLVMABI_X86_64_CPUKIND_HPP

#include <string>

#include <llvm/ADT/Triple.h>

namespace llvm_abi {
	
	namespace x86 {
		
		/// \brief Enumeration of all of the X86 CPUs supported by Clang.
		///
		/// Each enumeration represents a particular CPU supported by Clang. These
		/// loosely correspond to the options passed to '-march' or '-mtune' flags.
		enum CPUKind {
			CK_Generic,

			/// \name i386
			/// i386-generation processors.
			//@{
			CK_i386,
			//@}

			/// \name i486
			/// i486-generation processors.
			//@{
			CK_i486,
			CK_WinChipC6,
			CK_WinChip2,
			CK_C3,
			//@}

			/// \name i586
			/// i586-generation processors, P5 microarchitecture based.
			//@{
			CK_i586,
			CK_Pentium,
			CK_PentiumMMX,
			//@}

			/// \name i686
			/// i686-generation processors, P6 / Pentium M microarchitecture based.
			//@{
			CK_i686,
			CK_PentiumPro,
			CK_Pentium2,
			CK_Pentium3,
			CK_Pentium3M,
			CK_PentiumM,
			CK_C3_2,

			/// This enumerator is a bit odd, as GCC no longer accepts -march=yonah.
			/// Clang however has some logic to suport this.
			// FIXME: Warn, deprecate, and potentially remove this.
			CK_Yonah,
			//@}

			/// \name Netburst
			/// Netburst microarchitecture based processors.
			//@{
			CK_Pentium4,
			CK_Pentium4M,
			CK_Prescott,
			CK_Nocona,
			//@}

			/// \name Core
			/// Core microarchitecture based processors.
			//@{
			CK_Core2,

			/// This enumerator, like \see CK_Yonah, is a bit odd. It is another
			/// codename which GCC no longer accepts as an option to -march, but Clang
			/// has some logic for recognizing it.
			// FIXME: Warn, deprecate, and potentially remove this.
			CK_Penryn,
			//@}

			/// \name Atom
			/// Atom processors
			//@{
			CK_Bonnell,
			CK_Silvermont,
			//@}

			/// \name Nehalem
			/// Nehalem microarchitecture based processors.
			CK_Nehalem,

			/// \name Westmere
			/// Westmere microarchitecture based processors.
			CK_Westmere,

			/// \name Sandy Bridge
			/// Sandy Bridge microarchitecture based processors.
			CK_SandyBridge,

			/// \name Ivy Bridge
			/// Ivy Bridge microarchitecture based processors.
			CK_IvyBridge,

			/// \name Haswell
			/// Haswell microarchitecture based processors.
			CK_Haswell,

			/// \name Broadwell
			/// Broadwell microarchitecture based processors.
			CK_Broadwell,

			/// \name Skylake
			/// Skylake microarchitecture based processors.
			CK_Skylake,

			/// \name Knights Landing
			/// Knights Landing processor.
			CK_KNL,

			/// \name K6
			/// K6 architecture processors.
			//@{
			CK_K6,
			CK_K6_2,
			CK_K6_3,
			//@}

			/// \name K7
			/// K7 architecture processors.
			//@{
			CK_Athlon,
			CK_AthlonThunderbird,
			CK_Athlon4,
			CK_AthlonXP,
			CK_AthlonMP,
			//@}

			/// \name K8
			/// K8 architecture processors.
			//@{
			CK_Athlon64,
			CK_Athlon64SSE3,
			CK_AthlonFX,
			CK_K8,
			CK_K8SSE3,
			CK_Opteron,
			CK_OpteronSSE3,
			CK_AMDFAM10,
			//@}

			/// \name Bobcat
			/// Bobcat architecture processors.
			//@{
			CK_BTVER1,
			CK_BTVER2,
			//@}

			/// \name Bulldozer
			/// Bulldozer architecture processors.
			//@{
			CK_BDVER1,
			CK_BDVER2,
			CK_BDVER3,
			CK_BDVER4,
			//@}

			/// This specification is deprecated and will be removed in the future.
			/// Users should prefer \see CK_K8.
			// FIXME: Warn on this when the CPU is set to it.
			//@{
			CK_x86_64,
			//@}

			/// \name Geode
			/// Geode processors.
			//@{
			CK_Geode
			//@}
		};
		
		std::string selectCPUName(const llvm::Triple& /*targetTriple*/,
		                          const std::string& cpu);
		
		CPUKind getCPUKind(const llvm::Triple& targetTriple,
		                   const std::string& userCPUString);
		
	}
	
}

#endif