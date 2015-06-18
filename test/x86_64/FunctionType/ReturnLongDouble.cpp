TEST(x86_64, ReturnLongDouble) {
	const auto functionType = getX86_64FnType(FunctionType(LongDoubleTy, {}));
	
	EXPECT_FALSE(functionType->isVarArg());
	EXPECT_TRUE(functionType->getReturnType()->isX86_FP80Ty());
	ASSERT_EQ(functionType->getNumParams(), 0);
}
