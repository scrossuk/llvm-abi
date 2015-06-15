#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Builder.hpp>
#include <llvm-abi/FunctionEncoder.hpp>
#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/Support.hpp>
#include <llvm-abi/Type.hpp>

#include "ABI_x86_64.hpp"
#include "ArgClass.hpp"
#include "Classification.hpp"
#include "TypeInfo.hpp"

namespace llvm_abi {
	
	namespace x86_64 {
		
		std::vector<size_t> getStructOffsets(llvm::ArrayRef<StructMember> structMembers) {
			std::vector<size_t> offsets;
			offsets.reserve(structMembers.size());
			
			size_t offset = 0;
			for (const auto& member: structMembers) {
				if (member.offset() < offset) {
					// Add necessary padding before this member.
					offset = roundUpToAlign(offset, getTypeAlign(member.type()));
				} else {
					offset = member.offset();
				}
				
				offsets.push_back(offset);
				
				// Add the member's size.
				offset += getTypeSize(member.type());
			}
			
			return offsets;
		}
		
		static bool isFloat32(const Type type) {
			return type.isFloatingPoint() && type.floatingPointKind() == Float;
		}
		
		// Returns the type to pass as, or null if no transformation is needed.
		llvm::Type* computeAbiType(llvm::LLVMContext& context, const Type type) {
			if (!((type.isComplex() && type.complexKind() == Float) || type.isStruct())) {
				return nullptr; // Nothing to do.
			}
			
			// TODO: refactor!
			if (type.isStruct() && type.structMembers().size() == 2
				&& type.structMembers().front().type().isPointer()
				&& type.structMembers().back().type().isInteger()
				&& type.structMembers().back().type().integerKind() == Int32) {
				// Nothing to do.
				return nullptr;
			}
			
			const auto classification = classify(type);
			if (classification.isMemory()) {
				// LLVM presumably handles passing values in memory correctly.
				return nullptr;
			}
			
			assert(!classification.isMemory());
			
			if (classification.classes[0] == NoClass) {
				assert(classification.classes[1] == NoClass && "Non-empty struct with empty first half?");
				return nullptr; // Empty structs should also be handled correctly by LLVM.
			}
			
			// Okay, we may need to transform. Figure out a canonical type:
			
			std::vector<llvm::Type*> parts;
			parts.reserve(2);
			
			const auto size = getTypeSize(type);
			
			switch (classification.classes[0]) {
				case Integer: {
					const auto bits = (size >= 8 ? 64 : (size * 8));
					parts.push_back(llvm::IntegerType::get(context, bits));
					break;
				}
				
				case Sse: {
					if (size <= 4) {
						parts.push_back(llvm::Type::getFloatTy(context));
					} else {
						if (type.isStruct()) {
							const auto& structMembers = type.structMembers();
							const auto firstMember = structMembers[0].type();
							if (firstMember.isFloatingPoint() && firstMember.floatingPointKind() == Float) {
								parts.push_back(llvm::VectorType::get(llvm::Type::getFloatTy(context), 2));
							} else {
								parts.push_back(llvm::Type::getDoubleTy(context));
							}
						} else {
							parts.push_back(llvm::Type::getDoubleTy(context));
						}
					}
					break;
				}
				
				case X87:
					assert(classification.classes[1] == X87Up && "Upper half of real not X87Up?");
					/// The type only contains a single real/ireal field,
					/// so just use that type.
					return llvm::Type::getX86_FP80Ty(context);
					
				default:
					llvm_unreachable("Unanticipated argument class.");
			}
			
			switch (classification.classes[1]) {
				case NoClass:
					assert(parts.size() == 1);
					// No need to use a single-element struct type.
					// Just use the element type instead.
					return parts.at(0);
					
				case Integer: {
					assert(size > 8);
					const auto bits = (size - 8) * 8;
					parts.push_back(llvm::IntegerType::get(context, bits));
					break;
				}
				
				case Sse: {
					if (size <= 12) {
						parts.push_back(llvm::Type::getFloatTy(context));
					} else {
						if (type.isStruct()) {
							const auto& structMembers = type.structMembers();
							const auto memberOffsets = getStructOffsets(structMembers);
							
							size_t memberIndex = 0;
							while (memberIndex < structMembers.size() && memberOffsets.at(memberIndex) < 8) {
								memberIndex++;
							}
							
							if (memberIndex != structMembers.size() && isFloat32(structMembers[memberIndex].type())) {
								parts.push_back(llvm::VectorType::get(llvm::Type::getFloatTy(context), 2));
							} else {
								parts.push_back(llvm::Type::getDoubleTy(context));
							}
						} else {
							parts.push_back(llvm::Type::getDoubleTy(context));
						}
					}
					break;
				}
				
				case X87Up:
					if (classification.classes[0] == X87) {
						// This won't happen: it was short-circuited while
						// processing the first half.
					} else {
						// I can't find this anywhere in the ABI documentation,
						// but this is what gcc does (both regular and llvm-gcc).
						// (This triggers for types like union { real r; byte b; })
						parts.push_back(llvm::Type::getDoubleTy(context));
					}
					
					break;
					
				default:
					llvm_unreachable("Unanticipated argument class for second half.");
			}
			
			return llvm::StructType::get(context, parts);
		}
		
	}
	
	ABI_x86_64::ABI_x86_64(llvm::Module* module)
	: llvmContext_(module->getContext()) {
		const auto i8PtrType = llvm::Type::getInt8PtrTy(llvmContext_);
		const auto i64Type = llvm::Type::getInt64Ty(llvmContext_);
		llvm::Type* types[] = { i8PtrType, i8PtrType, i64Type };
		memcpyIntrinsic_ = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::memcpy, types);
	}
	
	ABI_x86_64::~ABI_x86_64() { }
	
	std::string ABI_x86_64::name() const {
		return "x86_64";
	}
	
	size_t ABI_x86_64::typeSize(const Type type) const {
		const auto iterator = sizeOfCache_.find(type);
		if (iterator != sizeOfCache_.end()) {
			return iterator->second;
		}
		
		const auto value = x86_64::getTypeSize(type);
		sizeOfCache_.insert(std::make_pair(type, value));
		return value;
	}
	
	size_t ABI_x86_64::typeAlign(const Type type) const {
		const auto iterator = alignOfCache_.find(type);
		if (iterator != alignOfCache_.end()) {
			return iterator->second;
		}
		
		const auto value = x86_64::getTypeAlign(type);
		alignOfCache_.insert(std::make_pair(type, value));
		return value;
	}
	
	llvm::Type* ABI_x86_64::encodedType(Type type) const {
		const auto iterator = abiTypeCache_.find(type);
		if (iterator != abiTypeCache_.end()) {
			return iterator->second;
		}
		
		const auto llvmABIType = x86_64::computeAbiType(llvmContext_, type);
		abiTypeCache_.insert(std::make_pair(type, llvmABIType));
		return llvmABIType;
	}
	
	llvm::Type* ABI_x86_64::abiType(Type type) const {
		switch (type.kind()) {
			case VoidType:
				return llvm::Type::getVoidTy(llvmContext_);
			case PointerType:
				return llvm::Type::getInt8PtrTy(llvmContext_);
			case IntegerType: {
				return llvm::IntegerType::get(llvmContext_, typeSize(type) * 8);
			}
			case FloatingPointType: {
				switch (type.floatingPointKind()) {
					case Float:
						return llvm::Type::getFloatTy(llvmContext_);
					case Double:
						return llvm::Type::getDoubleTy(llvmContext_);
					case LongDouble:
						return llvm::Type::getX86_FP80Ty(llvmContext_);
					case Float128:
						return llvm::Type::getFP128Ty(llvmContext_);
				}
				llvm_unreachable("Unknown Float type kind.");
			}
			case ComplexType: {
				llvm_unreachable("TODO");
			}
			case StructType: {
				llvm::SmallVector<llvm::Type*, 8> members;
				for (const auto& structMember: type.structMembers()) {
					members.push_back(abiType(structMember.type()));
				}
				return llvm::StructType::get(llvmContext_, members);
			}
			case ArrayType: {
				return llvm::ArrayType::get(abiType(type.arrayElementType()),
				                            type.arrayElementCount());
			}
		}
	}
	
	std::vector<size_t> ABI_x86_64::calculateStructOffsets(llvm::ArrayRef<StructMember> structMembers) const {
		return x86_64::getStructOffsets(structMembers);
	}
	
	llvm::Type* ABI_x86_64::longDoubleType() const {
		return llvm::Type::getX86_FP80Ty(llvmContext_);
	}
	
	llvm::CallingConv::ID ABI_x86_64::getCallingConvention(const CallingConvention callingConvention) const {
		switch (callingConvention) {
			case CC_CDefault:
			case CC_CppDefault:
				return llvm::CallingConv::C;
			default:
				llvm_unreachable("Invalid calling convention for ABI.");
		}
	}
	
	llvm::FunctionType* ABI_x86_64::getFunctionType(const FunctionType& functionType) const {
		// TODO: this is much too simplistic!
		//       Needs to handle indirect return values, extend parameters etc.
		
		llvm::Type* returnType = abiType(functionType.returnType());
		
		llvm::SmallVector<llvm::Type*, 10> argTypes;
		for (const auto& argumentType: functionType.argumentTypes()) {
			const auto llvmEncodedType = encodedType(argumentType);
			argTypes.push_back(llvmEncodedType != nullptr ? llvmEncodedType : abiType(argumentType));
		}
		
		return llvm::FunctionType::get(returnType, argTypes, functionType.isVarArg());
	}
	
	static void encodeValues(ABI_x86_64& abi, Builder& builder, llvm::SmallVector<llvm::Value*, 8>& argValues, llvm::ArrayRef<Type> argTypes) {
		assert(argValues.size() == argTypes.size());
		
		for (size_t i = 0; i < argValues.size(); i++) {
			const auto argValue = argValues[i];
			const auto argType = argTypes[i];
			const auto llvmAbiType = abi.encodedType(argType);
			if (llvmAbiType == nullptr) {
				continue;
			}
			
			auto& entryBuilder = builder.getEntryBuilder();
			auto& currentBuilder = builder.getBuilder();
			
			const auto argValuePtr = entryBuilder.CreateAlloca(argValue->getType());
			const auto encodedValuePtr = entryBuilder.CreateAlloca(llvmAbiType);
			
			const auto i8PtrType = llvm::Type::getInt8PtrTy(abi.context());
			const auto i1Type = llvm::Type::getInt1Ty(abi.context());
			const auto i32Type = llvm::Type::getInt32Ty(abi.context());
			const auto i64Type = llvm::Type::getInt64Ty(abi.context());
			
			currentBuilder.CreateStore(argValue, argValuePtr);
			
			const auto sourceValue = currentBuilder.CreatePointerCast(argValuePtr, i8PtrType);
			const auto destValue = currentBuilder.CreatePointerCast(encodedValuePtr, i8PtrType);
			
			llvm::Value* args[] = { destValue, sourceValue,
				llvm::ConstantInt::get(i64Type, abi.typeSize(argType)),
				llvm::ConstantInt::get(i32Type, abi.typeAlign(argType)),
				llvm::ConstantInt::get(i1Type, 0) };
			currentBuilder.CreateCall(abi.memcpyIntrinsic(), args);
			
			argValues[i] = currentBuilder.CreateLoad(encodedValuePtr);
		}
	}
	
	static void decodeValues(ABI_x86_64& abi, Builder& builder, llvm::SmallVector<llvm::Value*, 8>& argValues, llvm::ArrayRef<Type> argTypes/*, llvm::ArrayRef<llvm::Type*> llvmArgTypes*/) {
		assert(argValues.size() == argTypes.size());
		
		for (size_t i = 0; i < argValues.size(); i++) {
			const auto encodedValue = argValues[i];
			const auto argType = argTypes[i];
			const auto llvmAbiType = abi.encodedType(argType);
			if (llvmAbiType == nullptr) {
				continue;
			}
			
			auto& entryBuilder = builder.getEntryBuilder();
			auto& currentBuilder = builder.getBuilder();
			
			const auto encodedValuePtr = entryBuilder.CreateAlloca(encodedValue->getType());
			
// 			assert(llvmArgTypes[i] != nullptr);
// 			const auto argValuePtr = entryBuilder.CreateAlloca(llvmArgTypes[i]);
			const auto argValuePtr = entryBuilder.CreateAlloca(llvmAbiType);
			
			const auto i8PtrType = llvm::Type::getInt8PtrTy(abi.context());
			const auto i1Type = llvm::Type::getInt1Ty(abi.context());
			const auto i32Type = llvm::Type::getInt32Ty(abi.context());
			const auto i64Type = llvm::Type::getInt64Ty(abi.context());
			
			currentBuilder.CreateStore(encodedValue, encodedValuePtr);
			
			const auto sourceValue = currentBuilder.CreatePointerCast(encodedValuePtr, i8PtrType);
			const auto destValue = currentBuilder.CreatePointerCast(argValuePtr, i8PtrType);
			
			llvm::Value* args[] = { destValue, sourceValue,
				llvm::ConstantInt::get(i64Type, abi.typeSize(argType)),
				llvm::ConstantInt::get(i32Type, abi.typeAlign(argType)),
				llvm::ConstantInt::get(i1Type, 0) };
			currentBuilder.CreateCall(abi.memcpyIntrinsic(), args);
			
			argValues[i] = currentBuilder.CreateLoad(argValuePtr);
		}
	}
	
	llvm::Value* ABI_x86_64::createCall(Builder& builder,
	                                    const FunctionType& functionType,
	                                    std::function<llvm::Value* (llvm::ArrayRef<llvm::Value*>)> callBuilder,
	                                    llvm::ArrayRef<llvm::Value*> arguments) {
		llvm::SmallVector<llvm::Value*, 8> argumentValues(arguments.begin(), arguments.end());
		encodeValues(*this, builder, argumentValues, functionType.argumentTypes());
		
		const auto returnValue = callBuilder(argumentValues);
		
		llvm::SmallVector<llvm::Value*, 8> returnValues;
		returnValues.push_back(returnValue);
		Type returnTypes[] = { functionType.returnType() };
		decodeValues(*this, builder, returnValues, returnTypes);
		return returnValues[0];
	}
	
	class FunctionEncoder_x86_64: public FunctionEncoder {
	public:
		FunctionEncoder_x86_64(ABI_x86_64& abi,
		                       Builder& builder,
	                               const FunctionType& functionType,
		                       llvm::ArrayRef<llvm::Value*> pArguments)
		: abi_(abi),
		builder_(builder),
		functionType_(functionType),
		arguments_(pArguments.begin(), pArguments.end()) {
			decodeValues(abi_, builder_, arguments_, functionType.argumentTypes());
		}
		
		llvm::ArrayRef<llvm::Value*> arguments() const {
			return arguments_;
		}
		
		llvm::ReturnInst* returnValue(llvm::Value* const value) {
			llvm::SmallVector<llvm::Value*, 8> returnValues;
			returnValues.push_back(value);
			Type returnTypes[] = { functionType_.returnType() };
			encodeValues(abi_, builder_, returnValues, returnTypes);
			return builder_.getBuilder().CreateRet(returnValues[0]);
		}
		
	private:
		ABI_x86_64& abi_;
		Builder& builder_;
		const FunctionType& functionType_;
		llvm::SmallVector<llvm::Value*, 8> arguments_;
		
	};
	
	std::unique_ptr<FunctionEncoder> ABI_x86_64::createFunction(Builder& builder,
	                                                            const FunctionType& functionType,
	                                                            llvm::ArrayRef<llvm::Value*> arguments) {
		return std::unique_ptr<FunctionEncoder>(new FunctionEncoder_x86_64(*this,
		                                                               builder,
		                                                               functionType,
		                                                               arguments));
	}
	
}

