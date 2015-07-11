#include <llvm/IR/CallingConv.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm-abi/Type.hpp>
#include <llvm-abi/x86/X86_32Classifier.hpp>

namespace llvm_abi {
	
	namespace x86 {
		
		static bool isRegisterSize(const DataSize size) {
			return (size.asBits() == 8 || size.asBits() == 16 || size.asBits() == 32 || size.asBits() == 64);
		}
		
		X86_32Classifier::X86_32Classifier(const ABITypeInfo& typeInfo,
		                                   const TypeBuilder& typeBuilder,
		                                   const llvm::Triple targetTriple)
		: typeInfo_(typeInfo),
		typeBuilder_(typeBuilder),
		targetTriple_(targetTriple) {
			assert(targetTriple_.getArch() == llvm::Triple::x86);
		}
		
		bool X86_32Classifier::isDarwinVectorABI() const {
			return targetTriple_.isOSDarwin();
		}
		
		bool X86_32Classifier::isSmallStructInRegABI() const {
			assert(targetTriple_.getArch() == llvm::Triple::x86);
			
			// FIXME: have a way to pass in these options!
// 			switch (Opts.getStructReturnConvention()) {
// 			case CodeGenOptions::SRCK_Default:
// 				break;
// 			case CodeGenOptions::SRCK_OnStack:	// -fpcc-struct-return
// 				return false;
// 			case CodeGenOptions::SRCK_InRegs:	// -freg-struct-return
// 				return true;
// 			}
			
			if (targetTriple_.isOSDarwin()) {
				return true;
			}
			
			switch (targetTriple_.getOS()) {
				case llvm::Triple::DragonFly:
				case llvm::Triple::FreeBSD:
				case llvm::Triple::OpenBSD:
				case llvm::Triple::Bitrig:
				case llvm::Triple::Win32:
					return true;
				default:
					return false;
			}
		}
		
		bool X86_32Classifier::isWin32StructABI() const {
			return targetTriple_.isOSWindows() &&
			       !targetTriple_.isOSCygMing();
		}
		
		/// shouldReturnTypeInRegister - Determine if the given type should be
		/// passed in a register (for the Darwin ABI).
		bool X86_32Classifier::shouldReturnTypeInRegister(const Type type) const {
			const auto size = typeInfo_.getTypeAllocSize(type);

			// Type must be register sized.
			if (!isRegisterSize(size)) {
				return false;
			}

			if (type.isVector()) {
				// 64- and 128- bit vectors inside structures are not returned in
				// registers.
				if (size.asBits() == 64 || size.asBits() == 128) {
					return false;
				}
				
				return true;
			}
			
			// If this is a builtin, pointer, enum, complex type, member pointer, or
			// member function pointer it is ok.
			if (type.isInteger() || type.isFloat() || type.isPointer() || type.isComplex()) {
				return true;
			}
			
			// Arrays are treated like records.
			if (type.isArray()) {
				return shouldReturnTypeInRegister(type.arrayElementType());
			}
			
			// Otherwise, it must be a record type.
			if (!type.isStruct()) {
				return false;
			}
			
			// FIXME: Traverse bases here too.
			
			// Structure types are passed in register if all fields would be
			// passed in a register.
			for (const auto& member: type.structMembers()) {
				// Empty fields are ignored.
				if (member.isEmptyField(/*allowArrays=*/true)) {
					continue;
				}
				
				// Check fields recursively.
				if (!shouldReturnTypeInRegister(member.type())) {
					return false;
				}
			}
			
			return true;
		}

		ArgInfo X86_32Classifier::getIndirectReturnResult(CCState& state) const {
			// If the return value is indirect, then the hidden argument is consuming one
			// integer register.
			if (state.freeRegs > 0) {
				state.freeRegs--;
				return ArgInfo::getIndirectInReg(/*Align=*/0, /*ByVal=*/false);
			}
			return ArgInfo::getIndirect(/*Align=*/0, /*ByVal=*/false);
		}
		
		ArgInfo X86_32Classifier::classifyReturnType(const Type returnType,
		                                             CCState& state) const {
			if (returnType.isVoid()) {
				return ArgInfo::getIgnore();
			}
			
			Type base = VoidTy;
			uint64_t numElements = 0;
			if (state.callingConvention == CC_VectorCall &&
			    returnType.isHomogeneousAggregate(typeInfo_, base, numElements)) {
				// The LLVM struct type for such an aggregate should lower properly.
				return ArgInfo::getDirect(returnType);
			}
			
			if (returnType.isVector()) {
				// On Darwin, some vectors are returned in registers.
				if (isDarwinVectorABI()) {
					const auto size = typeInfo_.getTypeAllocSize(returnType);

					// 128-bit vectors are a special case; they are returned in
					// registers and we need to make sure to pick a type the LLVM
					// backend will like.
					if (size.asBits() == 128) {
						return ArgInfo::getDirect(typeBuilder_.getVectorTy(2, Int64Ty));
					}
					
					// Always return in register if it fits in a general purpose
					// register, or if it is 64 bits and has a single element.
					if ((size.asBits() == 8 || size.asBits() == 16 || size.asBits() == 32) ||
					    (size.asBits() == 64 && returnType.vectorElementCount() == 1)) {
						return ArgInfo::getDirect(Type::FixedWidthInteger(size, /*isSigned=*/false));
					}
					
					return getIndirectReturnResult(state);
				}
				
				return ArgInfo::getDirect(returnType);
			}
			
			if (returnType.isAggregateType()) {
				if (returnType.isStruct() && returnType.hasFlexibleArrayMember()) {
					// Structures with flexible arrays are always indirect.
					return getIndirectReturnResult(state);
				}
				
				// If specified, structs and unions are always indirect.
				if (!isSmallStructInRegABI() && !returnType.isComplex()) {
					return getIndirectReturnResult(state);
				}
				
				// Small structures which are register sized are generally returned
				// in a register.
				if (shouldReturnTypeInRegister(returnType)) {
					const auto size = typeInfo_.getTypeAllocSize(returnType);
					
					// As a special-case, if the struct is a "single-element" struct, and
					// the field is of type "float" or "double", return it in a
					// floating-point register. (MSVC does not apply this special case.)
					// We apply a similar transformation for pointer types to improve the
					// quality of the generated IR.
					const auto elementType = returnType.getStructSingleElement(typeInfo_);
					if (elementType != VoidTy) {
						if ((!isWin32StructABI() && elementType.isFloatingPoint())
						    || elementType.isPointer()) {
							return ArgInfo::getDirect(elementType);
						}
					}

					// FIXME: We should be able to narrow this integer in cases with dead
					// padding.
					const auto intType = Type::FixedWidthInteger(size, /*isSigned=*/false);
					return ArgInfo::getDirect(intType);
				}
				
				return getIndirectReturnResult(state);
			}
			
			return returnType.isPromotableIntegerType() ?
				ArgInfo::getExtend(returnType) :
				ArgInfo::getDirect(returnType);
		}
		
		bool X86_32Classifier::isSSEVectorType(const Type type) const {
			return type.isVector() && typeInfo_.getTypeAllocSize(type).asBits() == 128;
		}
		
		bool X86_32Classifier::isRecordWithSSEVectorType(const Type type) const {
			if (!type.isStruct()) {
				return false;
			}
			
			for (const auto& field: type.structMembers()) {
				const auto fieldType = field.type();
				if (isSSEVectorType(fieldType)) {
					return true;
				}
				
				if (isRecordWithSSEVectorType(fieldType)) {
					return true;
				}
			}
			
			return false;
		}
		
		static const DataSize MinABIStackAlign = DataSize::Bytes(4);
		
		DataSize
		X86_32Classifier::getTypeStackAlignInBytes(const Type type,
		                                           const DataSize align) const {
			// Otherwise, if the alignment is less than or equal to the minimum ABI
			// alignment, just use the default; the backend will handle this.
			if (align <= MinABIStackAlign) {
				return DataSize::Zero(); // Use default alignment.
			}
			
			// On non-Darwin, the stack type alignment is always 4.
			if (!isDarwinVectorABI()) {
				// Set explicit alignment, since we may need to realign the top.
				return MinABIStackAlign;
			}
			
			// Otherwise, if the type contains an SSE vector type, the alignment is 16.
			if (align.asBytes() >= 16 && (isSSEVectorType(type) ||
			    isRecordWithSSEVectorType(type))) {
				return DataSize::Bytes(16);
			}
			
			return MinABIStackAlign;
		}
		
		ArgInfo
		X86_32Classifier::getIndirectResult(const Type type,
		                                    const bool isByVal,
		                                    CCState& state) const {
			if (!isByVal) {
				if (state.freeRegs > 0) {
					state.freeRegs--; // Non-byval indirects just use one pointer.
					return ArgInfo::getIndirectInReg(0, false);
				}
				return ArgInfo::getIndirect(0, false);
			}
			
			// Compute the byval alignment.
			const auto typeAlign = typeInfo_.getTypeRequiredAlign(type);
			const auto stackAlign = getTypeStackAlignInBytes(type,
			                                                 typeAlign);
			if (stackAlign.asBytes() == 0) {
				return ArgInfo::getIndirect(4, /*ByVal=*/true);
			}
			
			// If the stack alignment is less than the type alignment,
			// realign the argument.
			const bool realign = typeAlign > stackAlign;
			return ArgInfo::getIndirect(stackAlign.asBytes(),
			                            /*ByVal=*/true,
			                            realign);
		}
		
		X86_32Classifier::Class
		X86_32Classifier::classify(const Type type) const {
			auto elementType = type.getStructSingleElement(typeInfo_);
			if (elementType == VoidTy) {
				elementType = type;
			}
			
			if (elementType == FloatTy || elementType == DoubleTy) {
				return Float;
			}
			
			return Integer;
		}
		
		bool X86_32Classifier::shouldUseInReg(const Type type,
		                                      CCState& state,
		                                      bool& needsPadding) const {
			needsPadding = false;
			const auto typeClass = classify(type);
			if (typeClass == Float) {
				return false;
			}
			
			const auto size = typeInfo_.getTypeAllocSize(type);
			const auto sizeInRegs = size.roundUpToAlign(DataSize::Bits(32)) / DataSize::Bits(32);
			
			if (sizeInRegs == 0) {
				return false;
			}
			
			if (sizeInRegs > state.freeRegs) {
				state.freeRegs = 0;
				return false;
			}
			
			state.freeRegs -= sizeInRegs;
			
			if (state.callingConvention == CC_FastCall ||
			    state.callingConvention == CC_VectorCall) {
				if (size.asBits() > 32) {
					return false;
				}
				
				if (type.isInteger() || type.isPointer()) {
					return true;
				}
				
				if (state.freeRegs > 0) {
					needsPadding = true;
				}
				
				return false;
			}
			
			return true;
		}
		
		bool X86_32Classifier::isX86_MMXType(const Type type) const {
			const auto irType = typeInfo_.getLLVMType(type);
			// Return true if the type is an MMX type <2 x i32>, <4 x i16>, or <8 x i8>.
			return irType->isVectorTy() && irType->getPrimitiveSizeInBits() == 64 &&
				llvm::cast<llvm::VectorType>(irType)->getElementType()->isIntegerTy() &&
				irType->getScalarSizeInBits() != 64;
		}
		
		bool X86_32Classifier::is32Or64BitBasicType(Type type) const {
			// Treat complex types as the element type.
			if (type.isComplex()) {
				type = type.complexFloatingPointType();
			}

			// Check for a type which we know has a simple scalar argument-passing
			// convention without any padding. (We're specifically looking for 32
			// and 64-bit integer and integer-equivalents, float, and double.)
			if (!type.isIntegralType()) {
				return false;
			}
			
			const auto size = typeInfo_.getTypeAllocSize(type);
			return size.asBits() == 32 || size.asBits() == 64;
		}
		
		bool X86_32Classifier::canExpandIndirectArgument(const Type type) const {
			// FIXME: This predicate is missing many cases, currently it just follows
			// llvm-gcc (checks that all fields are 32-bit or 64-bit primitive types). We
			// should probably make this smarter, or better yet make the LLVM backend
			// capable of handling it.
			
			// We can only expand structure types.
			if (!type.isStruct()) {
				return false;
			}
			
			DataSize size = DataSize::Zero();
			
			for (const auto& field: type.structMembers()) {
				if (!is32Or64BitBasicType(field.type())) {
					return false;
				}
				
				// FIXME: Reject bit-fields wholesale; there are two problems, we don't know
				// how to expand them yet, and the predicate for telling if a bitfield still
				// counts as "basic" is more complicated than what we were doing previously.
				if (field.isBitField()) {
					return false;
				}
				
				size += typeInfo_.getTypeAllocSize(field.type());
			}
			
			// Make sure there are not any holes in the struct.
			if (size != typeInfo_.getTypeAllocSize(type)) {
				return false;
			}
			
			return true;
		}
		
		ArgInfo X86_32Classifier::classifyArgumentType(const Type type,
		                                               CCState& state) const {
			// FIXME: Set alignment on indirect arguments.
			
			// vectorcall adds the concept of a homogenous vector aggregate, similar
			// to other targets.
			Type base = VoidTy;
			uint64_t numElements = 0;
			if (state.callingConvention == CC_VectorCall &&
			    type.isHomogeneousAggregate(typeInfo_, base, numElements)) {
				if (state.freeSSERegs >= numElements) {
					state.freeSSERegs -= numElements;
					if (type.isInteger() || type.isFloatingPoint() ||
					    type.isVector()) {
						return ArgInfo::getDirect(type);
					}
					return ArgInfo::getExpand(type);
				}
				return getIndirectResult(type,
				                         /*isByVal=*/false,
				                         state);
			}
			
			if (type.isAggregateType()) {
				if (type.isStruct()) {
					// Structs are always byval on win32, regardless of what they contain.
					if (isWin32StructABI()) {
						return getIndirectResult(type,
						                         /*isByVal=*/true,
						                         state);
					}
					
					// Structures with flexible arrays are always indirect.
					if (type.hasFlexibleArrayMember()) {
						return getIndirectResult(type,
						                         /*isByVal=*/true,
						                         state);
					}
				}
				
				// Ignore empty structs/unions.
				if (type.isEmptyRecord(/*allowArrays=*/true)) {
					return ArgInfo::getIgnore();
				}
				
				bool needsPadding;
				if (shouldUseInReg(type, state, needsPadding)) {
					const auto size = typeInfo_.getTypeAllocSize(type);
					const auto sizeInRegs = size.roundUpToAlign(DataSize::Bits(32)) / DataSize::Bits(32);
					llvm::SmallVector<Type, 3> elements(sizeInRegs, Int32Ty);
					const auto resultType = typeBuilder_.getStructTy(elements);
					return ArgInfo::getDirectInReg(resultType);
				}
				
				const auto paddingType = needsPadding ? Int32Ty : VoidTy;
				
				// Expand small (<= 128-bit) record types when we know that the stack layout
				// of those arguments will match the struct. This is important because the
				// LLVM backend isn't smart enough to remove byval, which inhibits many
				// optimizations.
				if (typeInfo_.getTypeAllocSize(type).asBits() <= (4 * 32) &&
				    canExpandIndirectArgument(type)) {
					return ArgInfo::getExpandWithPadding(
						type,
						state.callingConvention == CC_FastCall ||
						state.callingConvention == CC_VectorCall,
						paddingType);
				}
				
				return getIndirectResult(type,
				                         /*isByVal=*/true,
				                         state);
			}
			
			if (type.isVector()) {
				// On Darwin, some vectors are passed in memory, we handle this by passing
				// it as an i8/i16/i32/i64.
				if (isDarwinVectorABI()) {
					const auto size = typeInfo_.getTypeAllocSize(type);
					if ((size.asBits() == 8 || size.asBits() == 16 || size.asBits() == 32) ||
					    (size.asBits() == 64 && type.vectorElementCount() == 1)) {
						return ArgInfo::getDirect(Type::FixedWidthInteger(size, /*isSigned=*/false));
					}
				}
				
				if (isX86_MMXType(type)) {
					return ArgInfo::getDirect(Int64Ty);
				}
				
				return ArgInfo::getDirect(type);
			}
			
			bool needsPadding;
			const bool inReg = shouldUseInReg(type, state, needsPadding);
			if (type.isPromotableIntegerType()) {
				if (inReg) {
					return ArgInfo::getExtendInReg(type);
				}
				return ArgInfo::getExtend(type);
			}
			if (inReg) {
				return ArgInfo::getDirectInReg(type);
			}
			return ArgInfo::getDirect(type);
		}
		
		llvm::SmallVector<ArgInfo, 8>
		X86_32Classifier::classifyFunctionType(const FunctionType& functionType,
		                                       llvm::ArrayRef<Type> argumentTypes) const {
			// FIXME: This needs to be user-configurable; by default
			// we don't pass arguments in registers but users can
			// enable this.
			const auto DefaultNumRegisterParameters = 0;
			
			CCState state(functionType.callingConvention());
			if (state.callingConvention == CC_FastCall) {
				state.freeRegs = 2;
			} else if (state.callingConvention == CC_VectorCall) {
				state.freeRegs = 2;
				state.freeSSERegs = 6;
// 			} else if (FI.getHasRegParm()) {
// 				state.freeRegs = FI.getRegParm();
			} else {
				state.freeRegs = DefaultNumRegisterParameters;
			}
			
			llvm::SmallVector<ArgInfo, 8> argInfoArray;
			
			argInfoArray.push_back(classifyReturnType(functionType.returnType(),
			                                          state));
			
			// The chain argument effectively gives us another free register.
// 			if (FI.isChainCall()) {
// 				state.freeRegs++;
// 			}
			
			bool usedInAlloca = false;
			for (const auto& argumentType: argumentTypes) {
				const auto argInfo = classifyArgumentType(argumentType, state);
				usedInAlloca |= (argInfo.getKind() == ArgInfo::InAlloca);
				argInfoArray.push_back(argInfo);
			}
			
			// If we needed to use inalloca for any argument, do a second pass and rewrite
			// all the memory arguments to use inalloca.
			// TODO!
			(void) usedInAlloca;
// 			if (UsedInAlloca)
// 				rewriteWithInAlloca(FI);
			return argInfoArray;
		}
		
	}
	
}

