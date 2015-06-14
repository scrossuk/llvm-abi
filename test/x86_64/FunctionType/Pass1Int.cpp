TEST(x86_64, Pass1Int) {
	llvm::LLVMContext context;
	llvm::Module module("test", context);
	
	llvm_abi::Context abiContext;
	
	const auto voidType = llvm_abi::Type::Void(abiContext);
	const auto intType = llvm_abi::Type::Integer(abiContext, llvm_abi::Int);
	
	const llvm_abi::Type* argTypes[] = { intType };
	
	llvm_abi::FunctionType functionType(voidType, argTypes);
	
	const auto llvmFunctionType = createX86_64ABI(module)->getFunctionType(functionType);
	
	EXPECT_FALSE(llvmFunctionType->isVarArg());
	EXPECT_TRUE(llvmFunctionType->getReturnType()->isVoidTy());
	ASSERT_EQ(llvmFunctionType->getNumParams(), 1);
	EXPECT_TRUE(llvmFunctionType->getParamType(0)->isIntegerTy(32));
}
