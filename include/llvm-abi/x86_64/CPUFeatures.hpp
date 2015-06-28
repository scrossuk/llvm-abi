#ifndef LLVMABI_X86_64_CPUFEATURES_HPP
#define LLVMABI_X86_64_CPUFEATURES_HPP

#include <set>
#include <string>

#include <llvm/ADT/Triple.h>

#include <llvm-abi/x86_64/CPUKind.hpp>

namespace llvm_abi {
	
	namespace x86_64 {
		
		enum SSELevel {
			NoSSE,
			SSE1,
			SSE2,
			SSE3,
			SSSE3,
			SSE41,
			SSE42,
			AVX,
			AVX2,
			AVX512F
		};
		
		class CPUFeatures {
		public:
			CPUFeatures();
			
			void add(const std::string& feature);
			
			bool hasAVX() const;
			
			SSELevel sseLevel() const;
			
		private:
			std::set<std::string> features_;
			SSELevel sseLevel_;
			
		};
		
		CPUFeatures getCPUFeatures(const llvm::Triple& targetTriple,
		                           const CPUKind cpu);
		
	}
	
}

#endif