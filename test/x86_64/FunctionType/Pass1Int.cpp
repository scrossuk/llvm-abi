TEST(x86_64, Pass1Int) {
	const auto functionType = getX86_64FnType(FunctionType(VoidTy, { IntTy }));
	
	EXPECT_FALSE(functionType->isVarArg());
	EXPECT_TRUE(functionType->getReturnType()->isVoidTy());
	ASSERT_EQ(functionType->getNumParams(), 1);
	EXPECT_TRUE(functionType->getParamType(0)->isIntegerTy(32));
}
