#ifndef LLVMABI_X86_X86_32CLASSIFIER_HPP
#define LLVMABI_X86_X86_32CLASSIFIER_HPP

#include <llvm/ADT/Triple.h>

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/ArgInfo.hpp>
#include <llvm-abi/CallingConvention.hpp>
#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/TypeBuilder.hpp>

namespace llvm_abi {
	
	namespace x86 {
		
		struct CCState {
			CCState(const CallingConvention argCallingConvention) :
			callingConvention(argCallingConvention),
			freeRegs(0), freeSSERegs(0) {}
			
			CallingConvention callingConvention;
			unsigned freeRegs;
			unsigned freeSSERegs;
		};
		
		class X86_32Classifier {
		public:
			enum Class {
				Integer,
				Float
			};
			
			X86_32Classifier(const ABITypeInfo& typeInfo,
			                 const TypeBuilder& typeBuilder,
			                 llvm::Triple targetTriple);
			
			DataSize getTypeStackAlignInBytes(Type type,
			                                  DataSize align) const;
			
			ArgInfo getIndirectResult(Type type,
			                          bool isByVal,
			                          CCState& state) const;
			
			Class classify(Type type) const;
			
			bool shouldUseInReg(Type type,
			                    CCState& state,
			                    bool& needsPadding) const;
			
			ArgInfo classifyReturnType(Type returnType,
			                           CCState& state) const;
			
			ArgInfo classifyArgumentType(Type type,
			                             CCState& state) const;
			
			llvm::SmallVector<ArgInfo, 8>
			classifyFunctionType(const FunctionType& functionType,
			                     llvm::ArrayRef<Type> argumentTypes) const;
			
		private:
			const ABITypeInfo& typeInfo_;
			const TypeBuilder& typeBuilder_;
			bool isDarwinVectorABI_;
			bool isSmallStructInRegABI_;
			bool isWin32StructABI_;
			
		};
		
	}
	
}

#endif
