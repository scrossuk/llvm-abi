TEST(x86_64, PassStruct2Floats) {
	TypeBuilder typeBuilder;
	const auto structType = typeBuilder.getStructTy({ FloatTy, FloatTy });
	
	const auto functionType = getX86_64FnType(FunctionType(VoidTy, { structType }));
	
	EXPECT_FALSE(functionType->isVarArg());
	EXPECT_TRUE(functionType->getReturnType()->isVoidTy());
	
	// Struct of 2 floats passed as vector of 2 floats.
	ASSERT_EQ(functionType->getNumParams(), 1) << *functionType;
	EXPECT_TRUE(functionType->getParamType(0)->isVectorTy()) << *functionType;
	EXPECT_EQ(functionType->getParamType(0)->getVectorNumElements(), 2) << *functionType;
	EXPECT_TRUE(functionType->getParamType(0)->getVectorElementType()->isFloatTy()) << *functionType;
}
