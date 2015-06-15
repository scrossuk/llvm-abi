TEST(x86_64, PassStruct2Ints) {
	TypeBuilder typeBuilder;
	const auto structType = typeBuilder.getStructTy({ IntTy, IntTy });
	
	const auto functionType = getX86_64FnType(FunctionType(VoidTy, { structType }));
	
	EXPECT_FALSE(functionType->isVarArg());
	EXPECT_TRUE(functionType->getReturnType()->isVoidTy());
	
	// Two 32-bit integers are coerced into a single 64-bit value.
	ASSERT_EQ(functionType->getNumParams(), 1) << *functionType;
	EXPECT_TRUE(functionType->getParamType(0)->isIntegerTy(64)) << *functionType;
}
