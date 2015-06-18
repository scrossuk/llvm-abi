TEST(x86_64, ReturnShort) {
	const auto functionType = getX86_64FnType(FunctionType(ShortTy, {}));
	
	EXPECT_FALSE(functionType->isVarArg());
	EXPECT_TRUE(functionType->getReturnType()->isIntegerTy(16));
	ASSERT_EQ(functionType->getNumParams(), 0);
}
