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
			
			bool isDarwinVectorABI() const;
			bool isSmallStructInRegABI() const;
			bool isWin32StructABI() const;
			
			bool shouldReturnTypeInRegister(Type type) const;
			
			ArgInfo getIndirectReturnResult(CCState& state) const;
			
			bool isSSEVectorType(Type type) const;
			
			bool isRecordWithSSEVectorType(Type type) const;
			
			DataSize getTypeStackAlignInBytes(Type type,
			                                  DataSize align) const;
			
			ArgInfo getIndirectResult(Type type,
			                          bool isByVal,
			                          CCState& state) const;
			
			Class classify(Type type) const;
			
			bool shouldUseInReg(Type type,
			                    CCState& state,
			                    bool& needsPadding) const;
			
			/// IsX86_MMXType - Return true if this is an MMX type.
			bool isX86_MMXType(Type type) const;
			
			bool is32Or64BitBasicType(Type type) const;
			
			/// canExpandIndirectArgument - Test whether an argument type which is to be
			/// passed indirectly (on the stack) would have the equivalent layout if it was
			/// expanded into separate arguments. If so, we prefer to do the latter to avoid
			/// inhibiting optimizations.
			bool canExpandIndirectArgument(Type type) const;
			
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
			llvm::Triple targetTriple_;
			
		};
		
	}
	
}

#endif
