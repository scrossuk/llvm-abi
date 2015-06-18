TEST(x86_64, ReturnInt) {
	const auto functionType = getX86_64FnType(FunctionType(IntTy, {}));
	
	EXPECT_FALSE(functionType->isVarArg());
	EXPECT_TRUE(functionType->getReturnType()->isIntegerTy(32));
	ASSERT_EQ(functionType->getNumParams(), 0);
}
