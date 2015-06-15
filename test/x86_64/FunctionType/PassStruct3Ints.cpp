TEST(x86_64, PassStruct3Ints) {
	TypeBuilder typeBuilder;
	const auto structType = typeBuilder.getStructTy({ IntTy, IntTy, IntTy });
	
	const auto functionType = getX86_64FnType(FunctionType(VoidTy, { structType }));
	
	EXPECT_FALSE(functionType->isVarArg());
	EXPECT_TRUE(functionType->getReturnType()->isVoidTy());
	
	// The first two 32-bit integers are coerced into a  64-bit value and the
	// last is passed directly.
	ASSERT_EQ(functionType->getNumParams(), 2) << *functionType;
	EXPECT_TRUE(functionType->getParamType(0)->isIntegerTy(64)) << *functionType;
	EXPECT_TRUE(functionType->getParamType(1)->isIntegerTy(32)) << *functionType;
}
