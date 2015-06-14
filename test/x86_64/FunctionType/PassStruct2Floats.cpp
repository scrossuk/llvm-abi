TEST(x86_64, PassStruct2Floats) {
	llvm::LLVMContext context;
	llvm::Module module("test", context);
	
	llvm_abi::Context abiContext;
	
	const auto voidType = llvm_abi::Type::Void(abiContext);
	const auto floatType = llvm_abi::Type::FloatingPoint(abiContext, llvm_abi::Float);
	
	const llvm_abi::Type* memberTypes[] = { floatType, floatType };
	const auto structType = llvm_abi::Type::AutoStruct(abiContext, memberTypes);
	
	const llvm_abi::Type* argTypes[] = { structType };
	llvm_abi::FunctionType functionType(voidType, argTypes);
	
	const auto llvmFunctionType = createX86_64ABI(module)->getFunctionType(functionType);
	
	EXPECT_FALSE(llvmFunctionType->isVarArg());
	EXPECT_TRUE(llvmFunctionType->getReturnType()->isVoidTy());
	
	// Struct of 2 floats passed as vector of 2 floats.
	ASSERT_EQ(llvmFunctionType->getNumParams(), 1) << *llvmFunctionType;
	EXPECT_TRUE(llvmFunctionType->getParamType(0)->isVectorTy()) << *llvmFunctionType;
	EXPECT_EQ(llvmFunctionType->getParamType(0)->getVectorNumElements(), 2) << *llvmFunctionType;
	EXPECT_TRUE(llvmFunctionType->getParamType(0)->getVectorElementType()->isFloatTy()) << *llvmFunctionType;
}
