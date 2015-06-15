TEST(x86_64, PassStruct1Float) {
	TypeBuilder typeBuilder;
	const auto structType = typeBuilder.getStructTy({ FloatTy });
	
	const auto functionType = getX86_64FnType(FunctionType(VoidTy, { structType }));
	
	EXPECT_FALSE(functionType->isVarArg());
	EXPECT_TRUE(functionType->getReturnType()->isVoidTy());
	
	ASSERT_EQ(functionType->getNumParams(), 1) << *functionType;
	EXPECT_TRUE(functionType->getParamType(0)->isFloatTy()) << *functionType;
}
