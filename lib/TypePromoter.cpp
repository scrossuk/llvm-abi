#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Builder.hpp>
#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypedValue.hpp>
#include <llvm-abi/TypePromoter.hpp>

namespace llvm_abi {
	
	TypePromoter::TypePromoter(const ABITypeInfo& typeInfo,
	                           Builder& builder)
	: typeInfo_(typeInfo),
	builder_(builder) { }
	
	TypedValue TypePromoter::promoteValue(llvm::Value* const value,
	                                      const Type type) {
		assert(type.isInteger() || type.isFloatingPoint());
		if (type.isInteger()) {
			if (type.hasSignedIntegerRepresentation(typeInfo_)) {
				const auto extValue = builder_.getBuilder().CreateSExt(value,
				                                                       typeInfo_.getLLVMType(type));
				return TypedValue(extValue, type);
			} else {
				const auto extValue = builder_.getBuilder().CreateZExt(value,
				                                                       typeInfo_.getLLVMType(type));
				return TypedValue(extValue, type);
			}
		} else {
			const auto extValue = builder_.getBuilder().CreateFPExt(value,
			                                                        typeInfo_.getLLVMType(type));
			return TypedValue(extValue, type);
		}
	}
	
	TypedValue TypePromoter::promoteVarArgsArgument(const TypedValue typedValue) {
		const auto value = typedValue.first;
		const auto type = typedValue.second;
		
		if (type.isUnspecifiedWidthInteger()) {
			switch (type.integerKind()) {
				case Char:
					return typeInfo_.isCharSigned() ?
						promoteValue(value, IntTy) :
						promoteValue(value, UIntTy);
				case Bool:
				case SChar:
				case Short:
					return promoteValue(value, IntTy);
				case UChar:
				case UShort:
					return promoteValue(value, UIntTy);
				case Int:
				case Long:
				case LongLong:
				case SSizeT:
				case IntPtrT:
				case UInt:
				case ULong:
				case ULongLong:
				case SizeT:
				case PtrDiffT:
				case UIntPtrT:
					return typedValue;
			}
		} else if (type.isFloatingPoint()) {
			switch (type.floatingPointKind()) {
				case Float:
					return promoteValue(value, DoubleTy);
				case Double:
				case LongDouble:
				case Float128:
					return typedValue;
			}
		} else {
			return typedValue;
		}
	}
	
	llvm::SmallVector<TypedValue, 8>
	TypePromoter::promoteArguments(const FunctionType& functionType,
	                               llvm::ArrayRef<TypedValue> arguments) {
		llvm::SmallVector<TypedValue, 8> promotedArguments;
		promotedArguments.reserve(arguments.size());
		
		for (size_t i = 0; i < arguments.size(); i++) {
			const bool isVarArgArgument = (i >= functionType.argumentTypes().size());
			if (isVarArgArgument) {
				promotedArguments.push_back(promoteVarArgsArgument(arguments[i]));
			} else {
				// Nothing to do.
				promotedArguments.push_back(arguments[i]);
			}
		}
		
		return promotedArguments;
	}
	
}
