TEST(x86_64, PassStruct4Ints) {
	TypeBuilder typeBuilder;
	const auto structType = typeBuilder.getStructTy({ IntTy, IntTy, IntTy, IntTy });
	
	const auto functionType = getX86_64FnType(FunctionType(VoidTy, { structType }));
	
	EXPECT_FALSE(functionType->isVarArg());
	EXPECT_TRUE(functionType->getReturnType()->isVoidTy());
	
	// Each pair of 32-bit ints are coerced to a 64-bit int.
	ASSERT_EQ(functionType->getNumParams(), 2) << *functionType;
	EXPECT_TRUE(functionType->getParamType(0)->isIntegerTy(64)) << *functionType;
	EXPECT_TRUE(functionType->getParamType(1)->isIntegerTy(64)) << *functionType;
}
