TEST(x86_64, PassStruct5Ints) {
	TypeBuilder typeBuilder;
	const auto structType = typeBuilder.getStructTy({ IntTy, IntTy, IntTy, IntTy, IntTy });
	
	const auto functionType = getX86_64FnType(FunctionType(VoidTy, { structType }));
	
	EXPECT_FALSE(functionType->isVarArg());
	EXPECT_TRUE(functionType->getReturnType()->isVoidTy());
	
	// Struct is passed in byval pointer.
	ASSERT_EQ(functionType->getNumParams(), 1) << *functionType;
	EXPECT_TRUE(functionType->getParamType(0)->isPointerTy()) << *functionType;
}
