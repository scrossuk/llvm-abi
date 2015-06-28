#include <memory>
#include <stdexcept>
#include <string>

#include <llvm-abi/ABI.hpp>

#include <llvm-abi/x86/ABI_Win64.hpp>
#include <llvm-abi/x86/ABI_x86.hpp>
#include <llvm-abi/x86/X86_64ABI.hpp>

namespace llvm_abi {
	
	std::unique_ptr<ABI> createABI(llvm::Module& module,
	                               const llvm::Triple& targetTriple,
	                               const std::string& cpuName) {
		switch (targetTriple.getArch()) {
			case llvm::Triple::x86:
				return std::unique_ptr<ABI>(new X86ABI(&module));
			case llvm::Triple::x86_64: {
				if (targetTriple.isOSWindows()) {
					return std::unique_ptr<ABI>(new Win64ABI(&module));
				} else {
					return std::unique_ptr<ABI>(new x86::X86_64ABI(&module,
					                                               targetTriple,
					                                               cpuName));
				}
			}
			default:
				break;
		}
		
		std::string errorString = "No ABI available for triple: ";
		errorString += targetTriple.str();
		throw std::runtime_error(errorString);
	}
	
}

