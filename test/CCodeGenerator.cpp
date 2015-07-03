#include <sstream>
#include <string>

#include <llvm/Support/ErrorHandling.h>

#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/Type.hpp>

#include "CCodeGenerator.hpp"
#include "TestFunctionType.hpp"

namespace llvm_abi {
	
	CCodeGenerator::CCodeGenerator()
	: arrayId_(0),
	functionId_(0),
	structId_(0),
	unionId_(0),
	vectorId_(0) {
	}

	std::string CCodeGenerator::generatedSourceCode() const {
		return sourceCodeStream_.str();
	}
	
	std::string CCodeGenerator::CCodeGenerator::emitType(const Type& type) {
		switch (type.kind()) {
			case VoidType:
				return "void";
			case PointerType:
				return "void*";
			case UnspecifiedWidthIntegerType:
				switch (type.integerKind()) {
					case Bool:
						return "bool";
					case Char:
						return "char";
					case SChar:
						return "signed char";
					case UChar:
						return "unsigned char";
					case Short:
						return "short";
					case UShort:
						return "unsigned short";
					case Int:
						return "int";
					case UInt:
						return "unsigned int";
					case Long:
						return "long";
					case ULong:
						return "unsigned long";
					case SizeT:
						return "size_t";
					case SSizeT:
						return "ssize_t";
					case PtrDiffT:
						return "ptrdiff_t";
					case IntPtrT:
						return "intptr_t";
					case UIntPtrT:
						return "uintptr_t";
					case LongLong:
						return "long long";
					case ULongLong:
						return "unsigned long long";
				}
				llvm_unreachable("Unknown integer type.");
			case FixedWidthIntegerType:
				switch (type.integerWidth().roundUpToPowerOf2Bytes().asBytes()) {
					case 1:
						return type.integerIsSigned() ?
						       "int8_t" : "uint8_t";
					case 2:
						return type.integerIsSigned() ?
						       "int16_t" : "uint16_t";
					case 4:
						return type.integerIsSigned() ?
						       "int32_t" : "uint32_t";
					case 8:
						return type.integerIsSigned() ?
						       "int64_t" : "uint64_t";
					default:
						llvm_unreachable("Unknown integer width.");
				}
			case FloatingPointType:
				switch (type.floatingPointKind()) {
					case Float:
						return "float";
					case Double:
						return "double";
					case LongDouble:
						return "long double";
					case Float128:
						return "__float128";
				}
				llvm_unreachable("Unknown floating point type.");
			case ComplexType:
				switch (type.complexKind()) {
					case Float:
						return "float _Complex";
					case Double:
						return "double _Complex";
					case LongDouble:
						return "long double _Complex";
					case Float128:
						return "_float128 _Complex";
				}
				llvm_unreachable("Unknown complex type.");
			case StructType: {
				std::vector<std::string> structMemberTypes;
				for (const auto& member: type.structMembers()) {
					structMemberTypes.push_back(emitType(member.type()));
				}
				
				sourceCodeStream_ << "typedef struct { ";
				
				size_t memberId = 0;
				for (const auto& memberType: structMemberTypes) {
					sourceCodeStream_ << memberType << " member" << memberId << "; ";
					memberId++;
				}
				
				sourceCodeStream_ << "}";
				
				sourceCodeStream_ << " Struct" << structId_ << ";" << std::endl;
				
				std::ostringstream stream;
				stream << "Struct" << structId_;
				structId_++;
				return stream.str();
			}
			case UnionType: {
				std::vector<std::string> unionMemberTypes;
				for (const auto& memberType: type.unionMembers()) {
					unionMemberTypes.push_back(emitType(memberType));
				}
				
				sourceCodeStream_ << "typedef union { ";
				
				size_t memberId = 0;
				for (const auto& memberType: unionMemberTypes) {
					sourceCodeStream_ << memberType << " member" << memberId << "; ";
					memberId++;
				}
				
				sourceCodeStream_ << "}";
				
				sourceCodeStream_ << " Union" << unionId_ << ";" << std::endl;
				
				std::ostringstream stream;
				stream << "Union" << unionId_;
				unionId_++;
				return stream.str();
			}
			case ArrayType: {
				const auto elementTypeString = emitType(type.arrayElementType());
				sourceCodeStream_ << "typedef ";
				sourceCodeStream_ << " struct { " << elementTypeString << " data[" << type.arrayElementCount() << "]; } Array" << arrayId_ << ";" << std::endl;
				std::ostringstream stream;
				stream << "Array" << arrayId_;
				arrayId_++;
				return stream.str();
			}
			case VectorType: {
				if (type.vectorElementType() == FloatTy) {
					const auto vectorSize = type.vectorElementCount() * 4;
					sourceCodeStream_ << "typedef float Vector" << vectorId_ << " ";
					sourceCodeStream_ << "__attribute__((__vector_size__(" << vectorSize << ")));" << std::endl;
					std::ostringstream stream;
					stream << "Vector" << vectorId_;
					vectorId_++;
					return stream.str();
				}
				
				llvm_unreachable("TODO");
			}
		}
	}
	
	size_t CCodeGenerator::emitFunctionTypes(const TestFunctionType& testFunctionType) {
		const auto& functionType = testFunctionType.functionType;
		const auto returnTypeString = emitType(functionType.returnType());
		sourceCodeStream_ << "typedef " << returnTypeString << " Fn" << functionId_ << "ReturnType;" << std::endl;
		int argId = 0;
		for (const auto& argType: functionType.argumentTypes()) {
			const auto argTypeString = emitType(argType);
			sourceCodeStream_ << "typedef " << argTypeString << " Fn" << functionId_ << "ArgType" << argId << ";" << std::endl;
			argId++;
		}
		int varArgId = 0;
		for (const auto& varArgType: testFunctionType.varArgsTypes) {
			const auto argTypeString = emitType(varArgType);
			sourceCodeStream_ << "typedef " << argTypeString << " Fn" << functionId_ << "VarArgType" << varArgId << ";" << std::endl;
			varArgId++;
		}
		sourceCodeStream_ << std::endl;
		
		return functionId_++;
	}
	
	void CCodeGenerator::emitCalleeFunction(const TestFunctionType& testFunctionType,
	                                        const size_t functionId) {
		const auto& functionType = testFunctionType.functionType;
		sourceCodeStream_ << "extern \"C\" Fn" << functionId << "ReturnType callee(";
		bool first = true;
		int argId = 0;
		for (const auto& argType: functionType.argumentTypes()) {
			(void) argType;
			if (first) {
				first = false;
			} else {
				sourceCodeStream_ << ", ";
			}
			sourceCodeStream_ << "Fn" << functionId << "ArgType" << argId << " arg" << argId;
			argId++;
		}
		
		if (functionType.isVarArg()) {
			assert(!functionType.argumentTypes().empty());
			sourceCodeStream_ << ", ...";
		}
		
		if (functionType.argumentTypes().empty()) {
			sourceCodeStream_ << "void";
		}
		sourceCodeStream_ << ");" << std::endl << std::endl;
	}
	
	void CCodeGenerator::emitCallerFunction(const TestFunctionType& testFunctionType,
	                                        const size_t functionId) {
		const auto& functionType = testFunctionType.functionType;
		sourceCodeStream_ << "extern \"C\" Fn" << functionId << "ReturnType caller(";
		bool first = true;
		int argId = 0;
		for (const auto& argType: functionType.argumentTypes()) {
			(void) argType;
			if (first) {
				first = false;
			} else {
				sourceCodeStream_ << ", ";
			}
			sourceCodeStream_ << "Fn" << functionId << "ArgType" << argId << " arg" << argId;
			argId++;
		}
		
		int varArgId = 0;
		for (const auto& varArgType: testFunctionType.varArgsTypes) {
			(void) varArgType;
			sourceCodeStream_ << ", ";
			sourceCodeStream_ << "Fn" << functionId << "VarArgType" << varArgId << " varArg" << varArgId;
			varArgId++;
		}
		
		sourceCodeStream_ << ") {" << std::endl;
		
		sourceCodeStream_ << "     return callee(";
		first = true;
		argId = 0;
		for (const auto& argType: functionType.argumentTypes()) {
			(void) argType;
			if (first) {
				first = false;
			} else {
				sourceCodeStream_ << ", ";
			}
			sourceCodeStream_ << "arg" << argId;
			argId++;
		}
		
		varArgId = 0;
		for (const auto& varArgType: testFunctionType.varArgsTypes) {
			(void) varArgType;
			sourceCodeStream_ << ", ";
			sourceCodeStream_ << "varArg" << varArgId;
			varArgId++;
		}
		
		sourceCodeStream_ << ");" << std::endl;
		sourceCodeStream_ << "}" << std::endl;
	}
	
	void CCodeGenerator::emitCalleeAndCallerFunctions(const TestFunctionType& functionType) {
		const auto functionId = emitFunctionTypes(functionType);
		emitCalleeFunction(functionType, functionId);
		emitCallerFunction(functionType, functionId);
	}
	
}
