#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Builder.hpp>
#include <llvm-abi/FunctionEncoder.hpp>
#include <llvm-abi/FunctionIRMapping.hpp>
#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/Support.hpp>
#include <llvm-abi/Type.hpp>

#include <llvm-abi/x86_64/ArgClass.hpp>
#include <llvm-abi/x86_64/Classification.hpp>
#include <llvm-abi/x86_64/Classifier.hpp>
#include <llvm-abi/x86_64/X86_64ABI.hpp>
#include <llvm-abi/x86_64/X86_64ABITypeInfo.hpp>

namespace llvm_abi {
	
	namespace x86_64 {
		
		static bool isFloat32(const Type type) {
			return type.isFloatingPoint() && type.floatingPointKind() == Float;
		}
		
		// Returns the type to pass as, or null if no transformation is needed.
		llvm::Type* computeAbiType(llvm::LLVMContext& context,
		                           const ABITypeInfo& typeInfo,
		                           const Type type) {
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
			
			const auto classification = Classifier(typeInfo).classify(type);
			if (classification.isMemory()) {
				// LLVM presumably handles passing values in memory correctly.
				return nullptr;
			}
			
			assert(!classification.isMemory());
			
			if (classification.low() == NoClass) {
				assert(classification.high() == NoClass && "Non-empty struct with empty first half?");
				return nullptr; // Empty structs should also be handled correctly by LLVM.
			}
			
			llvm::SmallVector<llvm::Type*, 2> parts;
			parts.reserve(2);
			
			const auto size = typeInfo.getTypeSize(type);
			
			switch (classification.low()) {
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
					assert(classification.high() == X87Up && "Upper half of real not X87Up?");
					/// The type only contains a single real/ireal field,
					/// so just use that type.
					return llvm::Type::getX86_FP80Ty(context);
					
				default:
					llvm_unreachable("Unanticipated argument class.");
			}
			
			switch (classification.high()) {
				case NoClass:
					assert(parts.size() == 1);
					// No need to use a single-element struct type.
					// Just use the element type instead.
					return parts[0];
					
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
							const auto memberOffsets = typeInfo.calculateStructOffsets(structMembers);
							
							size_t memberIndex = 0;
							while (memberIndex < structMembers.size() && memberOffsets[memberIndex] < 8) {
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
					if (classification.low() == X87) {
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
		
		X86_64ABI::X86_64ABI(llvm::Module* module)
		: llvmContext_(module->getContext()),
		typeInfo_(llvmContext_) {
			const auto i8PtrType = llvm::Type::getInt8PtrTy(llvmContext_);
			const auto i64Type = llvm::Type::getInt64Ty(llvmContext_);
			llvm::Type* types[] = { i8PtrType, i8PtrType, i64Type };
			memcpyIntrinsic_ = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::memcpy, types);
		}
		
		X86_64ABI::~X86_64ABI() { }
		
		std::string X86_64ABI::name() const {
			return "x86_64";
		}
		
		size_t X86_64ABI::typeSize(const Type type) const {
			const auto iterator = sizeOfCache_.find(type);
			if (iterator != sizeOfCache_.end()) {
				return iterator->second;
			}
			
			const auto value = typeInfo_.getTypeSize(type);
			sizeOfCache_.insert(std::make_pair(type, value));
			return value;
		}
		
		size_t X86_64ABI::typeAlign(const Type type) const {
			const auto iterator = alignOfCache_.find(type);
			if (iterator != alignOfCache_.end()) {
				return iterator->second;
			}
			
			const auto value = typeInfo_.getTypeAlign(type);
			alignOfCache_.insert(std::make_pair(type, value));
			return value;
		}
		
		llvm::Type* X86_64ABI::getLLVMType(const Type type) const {
			return typeInfo_.getLLVMType(type);
		}
		
		llvm::Type* X86_64ABI::encodedType(Type type) const {
			const auto iterator = abiTypeCache_.find(type);
			if (iterator != abiTypeCache_.end()) {
				return iterator->second;
			}
			
			const auto llvmABIType = computeAbiType(llvmContext_,
			                                        typeInfo_,
			                                        type);
			abiTypeCache_.insert(std::make_pair(type, llvmABIType));
			return llvmABIType;
		}
		
		std::vector<size_t> X86_64ABI::calculateStructOffsets(llvm::ArrayRef<StructMember> structMembers) const {
			auto result = typeInfo_.calculateStructOffsets(structMembers);
			return std::vector<size_t>(result.begin(), result.end());
		}
		
		llvm::Type* X86_64ABI::longDoubleType() const {
			return llvm::Type::getX86_FP80Ty(llvmContext_);
		}
		
		llvm::CallingConv::ID X86_64ABI::getCallingConvention(const CallingConvention callingConvention) const {
			switch (callingConvention) {
				case CC_CDefault:
				case CC_CppDefault:
					return llvm::CallingConv::C;
				default:
					llvm_unreachable("Invalid calling convention for ABI.");
			}
		}
		
		llvm::FunctionType* X86_64ABI::getFunctionType(const FunctionType& functionType) const {
			Classifier classifier(typeInfo_);
			const auto argInfoArray =
				classifier.classifyFunctionType(functionType);
			assert(argInfoArray.size() >= 1);
			
			const auto functionIRMapping = getFunctionIRMapping(argInfoArray);
			
			return llvm_abi::getFunctionType(llvmContext_,
			                                 typeInfo_,
			                                 functionType,
			                                 functionIRMapping);
		}
		
		static void encodeValues(X86_64ABI& abi, Builder& builder, llvm::SmallVector<llvm::Value*, 8>& argValues, llvm::ArrayRef<Type> argTypes) {
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
		
		static void decodeValues(X86_64ABI& abi, Builder& builder, llvm::SmallVector<llvm::Value*, 8>& argValues, llvm::ArrayRef<Type> argTypes/*, llvm::ArrayRef<llvm::Type*> llvmArgTypes*/) {
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
				
// 				assert(llvmArgTypes[i] != nullptr);
// 				const auto argValuePtr = entryBuilder.CreateAlloca(llvmArgTypes[i]);
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
		
		llvm::Value* X86_64ABI::createCall(Builder& builder,
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
			FunctionEncoder_x86_64(X86_64ABI& abi,
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
			
			llvm::Value* returnValuePointer() const {
				// TODO!
				return nullptr;
			}
			
		private:
			X86_64ABI& abi_;
			Builder& builder_;
			const FunctionType& functionType_;
			llvm::SmallVector<llvm::Value*, 8> arguments_;
			
		};
		
		std::unique_ptr<FunctionEncoder> X86_64ABI::createFunction(Builder& builder,
		                                                            const FunctionType& functionType,
		                                                            llvm::ArrayRef<llvm::Value*> arguments) {
			return std::unique_ptr<FunctionEncoder>(new FunctionEncoder_x86_64(*this,
			                                                               builder,
			                                                               functionType,
			                                                               arguments));
		}
		
	}
	
}

