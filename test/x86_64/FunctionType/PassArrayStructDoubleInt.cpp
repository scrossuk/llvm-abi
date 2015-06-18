TEST(x86_64, PassArrayStructDoubleInt) {
	TypeBuilder typeBuilder;
	const auto innerStructType = typeBuilder.getStructTy({ DoubleTy, IntTy });
	const auto arrayType = typeBuilder.getArrayTy(1, innerStructType);
	const auto structType = typeBuilder.getStructTy({ arrayType });
	
	const auto functionType = getX86_64FnType(FunctionType(VoidTy, { structType }));
	
	ASSERT_EQ(functionType->getNumParams(), 2) << *functionType;
	EXPECT_TRUE(functionType->getParamType(0)->isDoubleTy()) << *functionType;
	EXPECT_TRUE(functionType->getParamType(1)->isIntegerTy(32)) << *functionType;
}
