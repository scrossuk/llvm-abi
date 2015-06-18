TEST(x86_64, ReturnFloat) {
	const auto functionType = getX86_64FnType(FunctionType(FloatTy, {}));
	
	EXPECT_FALSE(functionType->isVarArg());
	EXPECT_TRUE(functionType->getReturnType()->isFloatTy());
	ASSERT_EQ(functionType->getNumParams(), 0);
}
