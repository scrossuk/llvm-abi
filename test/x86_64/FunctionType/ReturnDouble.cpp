TEST(x86_64, ReturnDouble) {
	const auto functionType = getX86_64FnType(FunctionType(DoubleTy, {}));
	
	EXPECT_FALSE(functionType->isVarArg());
	EXPECT_TRUE(functionType->getReturnType()->isDoubleTy());
	ASSERT_EQ(functionType->getNumParams(), 0);
}
