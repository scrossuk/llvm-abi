TEST(x86_64, PassCharShortIntLongLongPtr) {
	const auto functionType = getX86_64FnType(
		FunctionType(VoidTy, { CharTy, ShortTy, IntTy, LongLongTy, PointerTy })
	);
	
	EXPECT_FALSE(functionType->isVarArg());
	EXPECT_TRUE(functionType->getReturnType()->isVoidTy());
	ASSERT_EQ(functionType->getNumParams(), 5);
	EXPECT_TRUE(functionType->getParamType(0)->isIntegerTy(8)) << *functionType;
	EXPECT_TRUE(functionType->getParamType(1)->isIntegerTy(16)) << *functionType;
	EXPECT_TRUE(functionType->getParamType(2)->isIntegerTy(32)) << *functionType;
	EXPECT_TRUE(functionType->getParamType(3)->isIntegerTy(64)) << *functionType;
	EXPECT_TRUE(functionType->getParamType(4)->isPointerTy()) << *functionType;
}
