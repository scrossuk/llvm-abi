#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/DataSize.hpp>
#include <llvm-abi/DefaultABITypeInfo.hpp>

namespace llvm_abi {
	
	DefaultABITypeInfo::DefaultABITypeInfo(llvm::LLVMContext& llvmContext,
	                                       const ABITypeInfo& typeInfo,
	                                       const DefaultABITypeInfoDelegate& delegate)
	: llvmContext_(llvmContext),
	typeInfo_(typeInfo),
	delegate_(delegate) { }
	
	DefaultABITypeInfo::~DefaultABITypeInfo() { }
	
	DataSize
	DefaultABITypeInfo::getDefaultTypeRawSize(const Type type) const {
		switch (type.kind()) {
			case VoidType:
				return DataSize::Bytes(0);
			case PointerType:
				return delegate_.getPointerSize();
			case UnspecifiedWidthIntegerType:
				return delegate_.getIntSize(type.integerKind());
			case FixedWidthIntegerType:
				return type.integerWidth();
			case FloatingPointType:
				return delegate_.getFloatSize(type.floatingPointKind());
			case ComplexType:
				return delegate_.getComplexSize(type.complexKind());
			case StructType: {
				auto size = DataSize::Bytes(0);
				
				for (const auto& member: type.structMembers()) {
					if (member.offset() < size) {
						// Add necessary padding before this member.
						size = size.roundUpToAlign(typeInfo_.getTypeRequiredAlign(member.type()));
					} else {
						size = member.offset();
					}
					
					// Add the member's size.
					size += typeInfo_.getTypeAllocSize(member.type());
				}
				
				// Add any final padding.
				return size.roundUpToAlign(typeInfo_.getTypeRequiredAlign(type));
			}
			case UnionType: {
				auto size = DataSize::Bytes(0);
				
				for (const auto& member: type.unionMembers()) {
					const auto memberSize = typeInfo_.getTypeAllocSize(member.type());
					size = std::max<DataSize>(size, memberSize);
				}
				
				// Add any final padding.
				return size.roundUpToAlign(typeInfo_.getTypeRequiredAlign(type));
			}
			case ArrayType:
				// TODO: this is probably wrong...
				return typeInfo_.getTypeRawSize(type.arrayElementType()) * type.arrayElementCount();
			case VectorType:
				// TODO: this is probably wrong...
				return typeInfo_.getTypeRawSize(type.vectorElementType()) * type.vectorElementCount();
		}
		llvm_unreachable("Unknown ABI type.");
	}
	
	DataSize
	DefaultABITypeInfo::getDefaultTypeAllocSize(const Type type) const {
		if (type.isFixedWidthInteger()) {
			return type.integerWidth().roundUpToPowerOf2Bytes();
		}
		
		return typeInfo_.getTypeRawSize(type).roundUpToAlign(DataSize::Bytes(1));
	}
	
	DataSize
	DefaultABITypeInfo::getDefaultTypeStoreSize(const Type type) const {
		return typeInfo_.getTypeRawSize(type).roundUpToAlign(DataSize::Bytes(1));
	}
	
	DataSize
	DefaultABITypeInfo::getDefaultTypeRequiredAlign(const Type type) const {
		switch (type.kind()) {
			case VoidType:
				return DataSize::Bytes(0);
			case PointerType:
				return delegate_.getPointerAlign();
			case UnspecifiedWidthIntegerType: {
				return delegate_.getIntAlign(type.integerKind());
			}
			case FixedWidthIntegerType: {
				return type.integerWidth().roundUpToPowerOf2Bytes();
			}
			case FloatingPointType: {
				return delegate_.getFloatAlign(type.floatingPointKind());
			}
			case ComplexType: {
				return delegate_.getComplexAlign(type.complexKind());
			}
			case StructType: {
				auto mostStrictAlign = DataSize::Bytes(1);
				for (const auto& member: type.structMembers()) {
					const auto align = typeInfo_.getTypeRequiredAlign(member.type());
					mostStrictAlign = std::max<DataSize>(mostStrictAlign, align);
				}
				
				return mostStrictAlign;
			}
			case UnionType: {
				auto mostStrictAlign = DataSize::Bytes(1);
				for (const auto& member: type.unionMembers()) {
					const auto align = typeInfo_.getTypeRequiredAlign(member.type());
					mostStrictAlign = std::max<DataSize>(mostStrictAlign, align);
				}
				
				return mostStrictAlign;
			}
			case ArrayType: {
				return delegate_.getArrayAlign(type);
			}
			case VectorType: {
				return delegate_.getVectorAlign(type);
			}
		}
		llvm_unreachable("Unknown type kind.");
	}
	
	DataSize
	DefaultABITypeInfo::getDefaultTypePreferredAlign(const Type type) const {
		return typeInfo_.getTypeRequiredAlign(type);
	}
	
	llvm::StructType*
	DefaultABITypeInfo::getLLVMStructType(const std::string& name,
	                                      llvm::ArrayRef<llvm::Type*> members) const {
		if (name.empty()) {
			return llvm::StructType::get(llvmContext_, members);
		}
		
		const auto iterator = structTypes_.find(name);
		if (iterator != structTypes_.end()) {
			return iterator->second;
		}
		
		const auto structType = llvm::StructType::create(llvmContext_, members,
		                                                 name);
		structTypes_.insert(std::make_pair(name, structType));
		return structType;
	}
	
	llvm::Type*
	DefaultABITypeInfo::getDefaultLLVMType(const Type type) const {
		switch (type.kind()) {
			case VoidType:
				return llvm::Type::getVoidTy(llvmContext_);
			case PointerType:
				return llvm::Type::getInt8PtrTy(llvmContext_);
			case UnspecifiedWidthIntegerType:
			case FixedWidthIntegerType: {
				return llvm::IntegerType::get(llvmContext_,
				                              typeInfo_.getTypeRawSize(type).asBits());
			}
			case FloatingPointType: {
				switch (type.floatingPointKind()) {
					case HalfFloat:
						llvm_unreachable("TODO");
					case Float:
						return llvm::Type::getFloatTy(llvmContext_);
					case Double:
						return llvm::Type::getDoubleTy(llvmContext_);
					case LongDouble:
						return delegate_.getLongDoubleIRType();
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
					members.push_back(typeInfo_.getLLVMType(structMember.type()));
				}
				return getLLVMStructType(type.structName(), members);
			}
			case UnionType: {
				auto maxSize = DataSize::Bytes(0);
				llvm::Type* maxSizeLLVMType = nullptr;
				for (const auto& member: type.unionMembers()) {
					const auto size = typeInfo_.getTypeAllocSize(member.type());
					if (size > maxSize) {
						maxSize = size;
						maxSizeLLVMType = typeInfo_.getLLVMType(member.type());
					}
				}
				llvm::SmallVector<llvm::Type*, 1> members;
				if (maxSizeLLVMType != nullptr) {
					members.push_back(maxSizeLLVMType);
				}
				return getLLVMStructType(type.unionName(), members);
			}
			case ArrayType: {
				return llvm::ArrayType::get(typeInfo_.getLLVMType(type.arrayElementType()),
				                            type.arrayElementCount());
			}
			case VectorType: {
				return llvm::VectorType::get(typeInfo_.getLLVMType(type.vectorElementType()),
				                             type.vectorElementCount());
			}
		}
		
		llvm_unreachable("Unknown type for creating IR type.");
	}
	
	llvm::SmallVector<DataSize, 8>
	DefaultABITypeInfo::calculateDefaultStructOffsets(llvm::ArrayRef<RecordMember> structMembers) const {
		llvm::SmallVector<DataSize, 8> offsets;
		offsets.reserve(structMembers.size());
		
		auto offset = DataSize::Bytes(0);
		for (const auto& member: structMembers) {
			if (member.offset() < offset) {
				// Add necessary padding before this member.
				offset = offset.roundUpToAlign(typeInfo_.getTypeRequiredAlign(member.type()));
			} else {
				offset = member.offset();
			}
			
			offsets.push_back(offset);
			
			// Add the member's size.
			offset += typeInfo_.getTypeAllocSize(member.type());
		}
		
		return offsets;
	}

}
