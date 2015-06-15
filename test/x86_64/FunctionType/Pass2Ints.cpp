TEST(x86_64, Pass2Ints) {
	const auto functionType = getX86_64FnType(FunctionType(VoidTy, { IntTy, IntTy }));
	
	EXPECT_FALSE(functionType->isVarArg());
	EXPECT_TRUE(functionType->getReturnType()->isVoidTy());
	ASSERT_EQ(functionType->getNumParams(), 2);
	EXPECT_TRUE(functionType->getParamType(0)->isIntegerTy(32));
	EXPECT_TRUE(functionType->getParamType(1)->isIntegerTy(32));
}
