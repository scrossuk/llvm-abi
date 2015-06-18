TEST(x86_64, ReturnChar) {
	const auto functionType = getX86_64FnType(FunctionType(CharTy, {}));
	
	EXPECT_FALSE(functionType->isVarArg()) << *functionType;
	EXPECT_TRUE(functionType->getReturnType()->isIntegerTy(8)) << *functionType;
	ASSERT_EQ(functionType->getNumParams(), 0) << *functionType;
}
