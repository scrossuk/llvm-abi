#include <llvm/ADT/StringSwitch.h>
#include <llvm/ADT/Triple.h>

#include <llvm-abi/x86_64/CPUKind.hpp>

namespace llvm_abi {
	
	namespace x86_64 {
		
		std::string selectCPUName(const llvm::Triple& /*targetTriple*/,
		                          const std::string& cpu) {
			// TODO!
			return !cpu.empty() ? cpu : "x86-64";
		}
		
		CPUKind getCPUKind(const llvm::Triple& targetTriple,
		                   const std::string& userCPUString) {
			const auto cpuString = selectCPUName(targetTriple,
			                                     userCPUString);
			const auto cpu = llvm::StringSwitch<CPUKind>(cpuString)
				.Case("i386", CK_i386)
				.Case("i486", CK_i486)
				.Case("winchip-c6", CK_WinChipC6)
				.Case("winchip2", CK_WinChip2)
				.Case("c3", CK_C3)
				.Case("i586", CK_i586)
				.Case("pentium", CK_Pentium)
				.Case("pentium-mmx", CK_PentiumMMX)
				.Case("i686", CK_i686)
				.Case("pentiumpro", CK_PentiumPro)
				.Case("pentium2", CK_Pentium2)
				.Case("pentium3", CK_Pentium3)
				.Case("pentium3m", CK_Pentium3M)
				.Case("pentium-m", CK_PentiumM)
				.Case("c3-2", CK_C3_2)
				.Case("yonah", CK_Yonah)
				.Case("pentium4", CK_Pentium4)
				.Case("pentium4m", CK_Pentium4M)
				.Case("prescott", CK_Prescott)
				.Case("nocona", CK_Nocona)
				.Case("core2", CK_Core2)
				.Case("penryn", CK_Penryn)
				.Case("bonnell", CK_Bonnell)
				.Case("atom", CK_Bonnell) // Legacy name.
				.Case("silvermont", CK_Silvermont)
				.Case("slm", CK_Silvermont) // Legacy name.
				.Case("nehalem", CK_Nehalem)
				.Case("corei7", CK_Nehalem) // Legacy name.
				.Case("westmere", CK_Westmere)
				.Case("sandybridge", CK_SandyBridge)
				.Case("corei7-avx", CK_SandyBridge) // Legacy name.
				.Case("ivybridge", CK_IvyBridge)
				.Case("core-avx-i", CK_IvyBridge) // Legacy name.
				.Case("haswell", CK_Haswell)
				.Case("core-avx2", CK_Haswell) // Legacy name.
				.Case("broadwell", CK_Broadwell)
				.Case("skylake", CK_Skylake)
				.Case("skx", CK_Skylake) // Legacy name.
				.Case("knl", CK_KNL)
				.Case("k6", CK_K6)
				.Case("k6-2", CK_K6_2)
				.Case("k6-3", CK_K6_3)
				.Case("athlon", CK_Athlon)
				.Case("athlon-tbird", CK_AthlonThunderbird)
				.Case("athlon-4", CK_Athlon4)
				.Case("athlon-xp", CK_AthlonXP)
				.Case("athlon-mp", CK_AthlonMP)
				.Case("athlon64", CK_Athlon64)
				.Case("athlon64-sse3", CK_Athlon64SSE3)
				.Case("athlon-fx", CK_AthlonFX)
				.Case("k8", CK_K8)
				.Case("k8-sse3", CK_K8SSE3)
				.Case("opteron", CK_Opteron)
				.Case("opteron-sse3", CK_OpteronSSE3)
				.Case("barcelona", CK_AMDFAM10)
				.Case("amdfam10", CK_AMDFAM10)
				.Case("btver1", CK_BTVER1)
				.Case("btver2", CK_BTVER2)
				.Case("bdver1", CK_BDVER1)
				.Case("bdver2", CK_BDVER2)
				.Case("bdver3", CK_BDVER3)
				.Case("bdver4", CK_BDVER4)
				.Case("x86-64", CK_x86_64)
				.Case("geode", CK_Geode)
				.Default(CK_Generic);
			
			// Perform any per-CPU checks necessary to determine if
			// this CPU is acceptable.
			switch (cpu) {
				case CK_Generic:
					throw std::runtime_error("No processor selected!");

				case CK_i386:
				case CK_i486:
				case CK_WinChipC6:
				case CK_WinChip2:
				case CK_C3:
				case CK_i586:
				case CK_Pentium:
				case CK_PentiumMMX:
				case CK_i686:
				case CK_PentiumPro:
				case CK_Pentium2:
				case CK_Pentium3:
				case CK_Pentium3M:
				case CK_PentiumM:
				case CK_Yonah:
				case CK_C3_2:
				case CK_Pentium4:
				case CK_Pentium4M:
				case CK_Prescott:
				case CK_K6:
				case CK_K6_2:
				case CK_K6_3:
				case CK_Athlon:
				case CK_AthlonThunderbird:
				case CK_Athlon4:
				case CK_AthlonXP:
				case CK_AthlonMP:
				case CK_Geode:
					// Only accept certain architectures when compiling in 32-bit mode.
					if (targetTriple.getArch() != llvm::Triple::x86) {
						throw std::runtime_error("Invalid arch for 32-bit mode.");
					}
					
					// Fallthrough
				case CK_Nocona:
				case CK_Core2:
				case CK_Penryn:
				case CK_Bonnell:
				case CK_Silvermont:
				case CK_Nehalem:
				case CK_Westmere:
				case CK_SandyBridge:
				case CK_IvyBridge:
				case CK_Haswell:
				case CK_Broadwell:
				case CK_Skylake:
				case CK_KNL:
				case CK_Athlon64:
				case CK_Athlon64SSE3:
				case CK_AthlonFX:
				case CK_K8:
				case CK_K8SSE3:
				case CK_Opteron:
				case CK_OpteronSSE3:
				case CK_AMDFAM10:
				case CK_BTVER1:
				case CK_BTVER2:
				case CK_BDVER1:
				case CK_BDVER2:
				case CK_BDVER3:
				case CK_BDVER4:
				case CK_x86_64:
					return cpu;
			}
			llvm_unreachable("Unhandled CPU kind");
		}
		
	}
	
}
