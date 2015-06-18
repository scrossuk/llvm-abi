TEST(x86_64, PassStructDoubleInt) {
	TypeBuilder typeBuilder;
	const auto structType = typeBuilder.getStructTy({ DoubleTy, IntTy });
	
	const auto functionType = getX86_64FnType(FunctionType(VoidTy, { structType }));
	
	EXPECT_FALSE(functionType->isVarArg());
	EXPECT_TRUE(functionType->getReturnType()->isVoidTy());
	
	ASSERT_EQ(functionType->getNumParams(), 2) << *functionType;
	EXPECT_TRUE(functionType->getParamType(0)->isDoubleTy()) << *functionType;
	EXPECT_TRUE(functionType->getParamType(1)->isIntegerTy(32)) << *functionType;
}
