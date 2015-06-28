#include <set>
#include <string>

#include <llvm/ADT/StringSwitch.h>
#include <llvm/ADT/Triple.h>

#include <llvm-abi/x86/CPUFeatures.hpp>
#include <llvm-abi/x86/CPUKind.hpp>

namespace llvm_abi {
	
	namespace x86_64 {
		
		CPUFeatures::CPUFeatures()
		: sseLevel_(NoSSE) { }
		
		void CPUFeatures::add(const std::string& feature) {
			(void) features_.insert(feature);
			
			const auto level = llvm::StringSwitch<SSELevel>(feature)
				.Case("avx512f", AVX512F)
				.Case("avx512cd", AVX512F)
				.Case("avx512er", AVX512F)
				.Case("avx512pf", AVX512F)
				.Case("avx512dq", AVX512F)
				.Case("avx512bw", AVX512F)
				.Case("avx512vl", AVX512F)
				.Case("avx2", AVX2)
				.Case("fma", AVX)
				.Case("avx", AVX)
				.Case("sse4", SSE42)
				.Case("sse4.2", SSE42)
				.Case("sse4.1", SSE41)
				.Case("ssse3", SSSE3)
				.Case("sse3", SSE3)
				.Case("aes", SSE2)
				.Case("pclmul", SSE2)
				.Case("sse2", SSE2)
				.Case("sse", SSE1)
				.Default(NoSSE);
			sseLevel_ = std::max(sseLevel_, level);
		}
		
		bool CPUFeatures::hasAVX() const {
			return sseLevel() >= AVX;
		}
		
		SSELevel CPUFeatures::sseLevel() const {
			return sseLevel_;
		}
		
		CPUFeatures getCPUFeatures(const llvm::Triple& targetTriple,
		                           const CPUKind cpu) {
			CPUFeatures features;
			
			// X86_64 always has SSE2.
			if (targetTriple.getArch() == llvm::Triple::x86_64) {
				features.add("sse2");
			}
			
			switch (cpu) {
				case CK_Generic:
				case CK_i386:
				case CK_i486:
				case CK_i586:
				case CK_Pentium:
				case CK_i686:
				case CK_PentiumPro:
					break;
				case CK_PentiumMMX:
				case CK_Pentium2:
				case CK_K6:
				case CK_WinChipC6:
					features.add("mmx");
					break;
				case CK_Pentium3:
				case CK_Pentium3M:
				case CK_C3_2:
					features.add("sse");
					break;
				case CK_PentiumM:
				case CK_Pentium4:
				case CK_Pentium4M:
				case CK_x86_64:
					features.add("sse2");
					break;
				case CK_Yonah:
				case CK_Prescott:
				case CK_Nocona:
					features.add("sse3");
					features.add("cx16");
					break;
				case CK_Core2:
				case CK_Bonnell:
					features.add("ssse3");
					features.add("cx16");
					break;
				case CK_Penryn:
					features.add("sse4.1");
					features.add("cx16");
					break;
				case CK_Skylake:
					features.add("avx512f");
					features.add("avx512cd");
					features.add("avx512dq");
					features.add("avx512bw");
					features.add("avx512vl");
					// FALLTHROUGH
				case CK_Broadwell:
					features.add("rdseed");
					features.add("adx");
					// FALLTHROUGH
				case CK_Haswell:
					features.add("avx2");
					features.add("lzcnt");
					features.add("bmi");
					features.add("bmi2");
					features.add("rtm");
					features.add("fma");
					// FALLTHROUGH
				case CK_IvyBridge:
					features.add("rdrnd");
					features.add("f16c");
					features.add("fsgsbase");
					// FALLTHROUGH
				case CK_SandyBridge:
					features.add("avx");
					// FALLTHROUGH
				case CK_Westmere:
				case CK_Silvermont:
					features.add("aes");
					features.add("pclmul");
					// FALLTHROUGH
				case CK_Nehalem:
					features.add("sse4.2");
					features.add("cx16");
					break;
				case CK_KNL:
					features.add("avx512f");
					features.add("avx512cd");
					features.add("avx512er");
					features.add("avx512pf");
					features.add("rdseed");
					features.add("adx");
					features.add("lzcnt");
					features.add("bmi");
					features.add("bmi2");
					features.add("rtm");
					features.add("fma");
					features.add("rdrnd");
					features.add("f16c");
					features.add("fsgsbase");
					features.add("aes");
					features.add("pclmul");
					features.add("cx16");
					break;
				case CK_K6_2:
				case CK_K6_3:
				case CK_WinChip2:
				case CK_C3:
					features.add("3dnow");
					break;
				case CK_Athlon:
				case CK_AthlonThunderbird:
				case CK_Geode:
					features.add("3dnowa");
					break;
				case CK_Athlon4:
				case CK_AthlonXP:
				case CK_AthlonMP:
					features.add("sse");
					features.add("3dnowa");
					break;
				case CK_K8:
				case CK_Opteron:
				case CK_Athlon64:
				case CK_AthlonFX:
					features.add("sse2");
					features.add("3dnowa");
					break;
				case CK_AMDFAM10:
					features.add("sse4a");
					features.add("lzcnt");
					features.add("popcnt");
					// FALLTHROUGH
				case CK_K8SSE3:
				case CK_OpteronSSE3:
				case CK_Athlon64SSE3:
					features.add("sse3");
					features.add("3dnowa");
					break;
				case CK_BTVER2:
					features.add("avx");
					features.add("aes");
					features.add("pclmul");
					features.add("bmi");
					features.add("f16c");
					// FALLTHROUGH
				case CK_BTVER1:
					features.add("ssse3");
					features.add("sse4a");
					features.add("lzcnt");
					features.add("popcnt");
					features.add("prfchw");
					features.add("cx16");
					break;
				case CK_BDVER4:
					features.add("avx2");
					features.add("bmi2");
					// FALLTHROUGH
				case CK_BDVER3:
					features.add("fsgsbase");
					// FALLTHROUGH
				case CK_BDVER2:
					features.add("bmi");
					features.add("fma");
					features.add("f16c");
					features.add("tbm");
					// FALLTHROUGH
				case CK_BDVER1:
					// xop implies avx, sse4a and fma4.
					features.add("xop");
					features.add("lzcnt");
					features.add("aes");
					features.add("pclmul");
					features.add("prfchw");
					features.add("cx16");
					break;
			}
			
			return features;
		}
		
	}
	
}
