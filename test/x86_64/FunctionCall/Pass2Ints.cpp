TEST(x86_64, Pass2Ints) {
	TestSystem testSystem(X86_64_TRIPLE);
	
	llvm::Value* arguments[] = {
		testSystem.getIntTestValue(),
		testSystem.getIntTestValue()
	};
	
	FunctionType functionType(VoidTy, { IntTy, IntTy });
	
	const auto callInfo =
		testSystem.generateCall(functionType,
		                        arguments,
		                        testSystem.getUndefValue());
	
	ASSERT_TRUE(llvm::isa<llvm::UndefValue>(callInfo.encodedReturnValue));
	
	ASSERT_EQ(callInfo.encodedArguments.size(), 2);
	
	ASSERT_TRUE(callInfo.encodedArguments[0]->getType()->isIntegerTy(32));
	ASSERT_EQ(callInfo.encodedArguments[0], arguments[0]);
	
	ASSERT_TRUE(callInfo.encodedArguments[1]->getType()->isIntegerTy(32));
	ASSERT_EQ(callInfo.encodedArguments[1], arguments[1]);
}
