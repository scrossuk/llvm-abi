TEST(x86_64, PassStructArray1Char3Chars) {
	TypeBuilder typeBuilder;
	const auto charArrayType = typeBuilder.getArrayTy(8, CharTy);
	const auto structType = typeBuilder.getStructTy({ charArrayType, CharTy, CharTy, CharTy });
	
	const auto functionType = getX86_64FnType(FunctionType(VoidTy, { structType }));
	
	EXPECT_FALSE(functionType->isVarArg());
	EXPECT_TRUE(functionType->getReturnType()->isVoidTy());
	
	ASSERT_EQ(functionType->getNumParams(), 2) << *functionType;
	EXPECT_TRUE(functionType->getParamType(0)->isIntegerTy(64)) << *functionType;
	EXPECT_TRUE(functionType->getParamType(1)->isIntegerTy(24)) << *functionType;
}
