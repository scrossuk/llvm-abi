#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Builder.hpp>
#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypedValue.hpp>
#include <llvm-abi/TypePromoter.hpp>

namespace llvm_abi {
	
	TypePromoter::TypePromoter(const ABITypeInfo& typeInfo)
	: typeInfo_(typeInfo) { }
	
	TypedValue TypePromoter::promoteValue(Builder& builder,
	                                      const TypedValue value,
	                                      const Type type) const {
		if (value.second == type) {
			// Nothing to do.
			return value;
		}
		
		assert(type.isInteger() || type.isFloatingPoint());
		if (type.isInteger()) {
			if (type.hasSignedIntegerRepresentation(typeInfo_)) {
				const auto extValue = builder.getBuilder().CreateSExt(value.first,
				                                                      typeInfo_.getLLVMType(type));
				return TypedValue(extValue, type);
			} else {
				const auto extValue = builder.getBuilder().CreateZExt(value.first,
				                                                      typeInfo_.getLLVMType(type));
				return TypedValue(extValue, type);
			}
		} else {
			const auto extValue = builder.getBuilder().CreateFPExt(value.first,
			                                                       typeInfo_.getLLVMType(type));
			return TypedValue(extValue, type);
		}
	}
	
	Type TypePromoter::promoteVarArgsArgumentType(const Type type) const {
		if (type.isUnspecifiedWidthInteger()) {
			switch (type.integerKind()) {
				case Char:
					return typeInfo_.isCharSigned() ?
						IntTy : UIntTy;
				case Bool:
				case SChar:
				case Short:
					return IntTy;
				case UChar:
				case UShort:
					return UIntTy;
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
					return type;
			}
		} else if (type.isFloatingPoint()) {
			switch (type.floatingPointKind()) {
				case Float:
					return DoubleTy;
				case Double:
				case LongDouble:
				case Float128:
					return type;
			}
		} else {
			return type;
		}
	}
	
	TypedValue TypePromoter::promoteVarArgsArgument(Builder& builder,
	                                                const TypedValue typedValue) const {
		const auto type = typedValue.second;
		return promoteValue(builder,
		                    typedValue,
		                    promoteVarArgsArgumentType(type));
	}
	
	llvm::SmallVector<Type, 8>
	TypePromoter::promoteArgumentTypes(const FunctionType& functionType,
	                                   llvm::ArrayRef<Type> argumentTypes) const {
		llvm::SmallVector<Type, 8> promotedArgumentTypes;
		promotedArgumentTypes.reserve(argumentTypes.size());
		
		for (size_t i = 0; i < argumentTypes.size(); i++) {
			const bool isVarArgArgument = (i >= functionType.argumentTypes().size());
			if (isVarArgArgument) {
				promotedArgumentTypes.push_back(promoteVarArgsArgumentType(argumentTypes[i]));
			} else {
				// Nothing to do.
				promotedArgumentTypes.push_back(argumentTypes[i]);
			}
		}
		
		return promotedArgumentTypes;
	}
	
	llvm::SmallVector<TypedValue, 8>
	TypePromoter::promoteArguments(Builder& builder,
	                               const FunctionType& functionType,
	                               llvm::ArrayRef<TypedValue> arguments) const {
		llvm::SmallVector<TypedValue, 8> promotedArguments;
		promotedArguments.reserve(arguments.size());
		
		for (size_t i = 0; i < arguments.size(); i++) {
			const bool isVarArgArgument = (i >= functionType.argumentTypes().size());
			if (isVarArgArgument) {
				promotedArguments.push_back(promoteVarArgsArgument(builder,
				                                                   arguments[i]));
			} else {
				// Nothing to do.
				promotedArguments.push_back(arguments[i]);
			}
		}
		
		return promotedArguments;
	}
	
}
