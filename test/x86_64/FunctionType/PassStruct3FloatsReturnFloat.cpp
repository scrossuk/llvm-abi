TEST(x86_64, PassStruct3FloatsReturnFloat) {
	TypeBuilder typeBuilder;
	const auto structType = typeBuilder.getStructTy({ FloatTy, FloatTy, FloatTy });
	
	const auto functionType = getX86_64FnType(FunctionType(FloatTy, { structType }));
	
	EXPECT_FALSE(functionType->isVarArg());
	EXPECT_TRUE(functionType->getReturnType()->isFloatTy());
	
	ASSERT_EQ(functionType->getNumParams(), 2) << *functionType;
	EXPECT_TRUE(functionType->getParamType(0)->isVectorTy()) << *functionType;
	EXPECT_EQ(functionType->getParamType(0)->getVectorNumElements(), 2) << *functionType;
	EXPECT_TRUE(functionType->getParamType(0)->getVectorElementType()->isFloatTy()) << *functionType;
	EXPECT_TRUE(functionType->getParamType(1)->isFloatTy()) << *functionType;
}
