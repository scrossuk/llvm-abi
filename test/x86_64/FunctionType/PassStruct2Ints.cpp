TEST(x86_64, PassStruct2Ints) {
	llvm::LLVMContext context;
	llvm::Module module("test", context);
	
	llvm_abi::Context abiContext;
	
	const auto voidType = llvm_abi::Type::Void(abiContext);
	const auto intType = llvm_abi::Type::Integer(abiContext, llvm_abi::Int);
	
	const llvm_abi::Type* memberTypes[] = { intType, intType };
	const auto structType = llvm_abi::Type::AutoStruct(abiContext, memberTypes);
	
	const llvm_abi::Type* argTypes[] = { structType };
	llvm_abi::FunctionType functionType(voidType, argTypes);
	
	const auto llvmFunctionType = createX86_64ABI(module)->getFunctionType(functionType);
	
	EXPECT_FALSE(llvmFunctionType->isVarArg());
	EXPECT_TRUE(llvmFunctionType->getReturnType()->isVoidTy());
	
	// Two 32-bit integers are coerced into a single 64-bit value.
	ASSERT_EQ(llvmFunctionType->getNumParams(), 1) << *llvmFunctionType;
	EXPECT_TRUE(llvmFunctionType->getParamType(0)->isIntegerTy(64)) << *llvmFunctionType;
}
