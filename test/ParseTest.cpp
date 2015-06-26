#include <fstream>
#include <sstream>

#include "TestSystem.hpp"
#include "TokenStream.hpp"
#include "TypeParser.hpp"

std::string getFileName(const std::string& path) {
	assert(!path.empty());
	size_t offset = path.size() - 1;
	
	while (offset > 1) {
		if (path[offset - 1] == '/') {
			break;
		}
		
		offset--;
	}
	
	return path.substr(offset);
}

std::string getBaseName(const std::string& fileName) {
	assert(!fileName.empty());
	size_t offset = 0;
	
	while (offset < fileName.size()) {
		if (fileName[offset] == '.') {
			break;
		}
		
		offset++;
	}
	
	return fileName.substr(0, offset);
}

size_t arrayId = 0;
size_t structId = 0;

std::string makeCType(const Type& type, std::ostringstream& sourceCodeStream) {
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
				case Short:
					return "short";
				case Int:
					return "int";
				case Long:
					return "long";
				case SizeT:
					return "size_t";
				case PtrDiffT:
					return "ptrdiff_t";
				case IntPtrT:
					return "intptr_t";
				case LongLong:
					return "long long";
			}
			llvm_unreachable("Unknown integer type.");
		case FixedWidthIntegerType:
			switch (type.integerWidth().roundUpToPowerOf2Bytes().asBytes()) {
				case 1:
					return "int8_t";
				case 2:
					return "int16_t";
				case 4:
					return "int32_t";
				case 8:
					return "int64_t";
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
			sourceCodeStream << "typedef struct { ";
			
			int memberId = 0;
			
			for (const auto& member: type.structMembers()) {
				sourceCodeStream << makeCType(member.type(), sourceCodeStream) << " member" << memberId << "; ";
				memberId++;
			}
			
			sourceCodeStream << "}";
			
			sourceCodeStream << " Struct" << structId << ";" << std::endl;
			
			std::ostringstream stream;
			stream << "Struct" << structId;
			structId++;
			return stream.str();
		}
		case ArrayType: {
			const auto elementTypeString = makeCType(type.arrayElementType(), sourceCodeStream);
			sourceCodeStream << "typedef ";
			sourceCodeStream << " std::array<" << elementTypeString << ", " << type.arrayElementCount() << "> Array" << arrayId << ";" << std::endl;
			std::ostringstream stream;
			stream << "Array" << arrayId;
			arrayId++;
			return stream.str();
		}
		case VectorType:
			llvm_unreachable("TODO");
	}
}

std::string makeClangCCode(const FunctionType& functionType) {
	std::ostringstream stream;
	
	stream << "#include <array>" << std::endl << std::endl;
	
	{
		const auto returnTypeString = makeCType(functionType.returnType(), stream);
		stream << "typedef " << returnTypeString << " ReturnType;" << std::endl;
		int argId = 0;
		for (const auto& argType: functionType.argumentTypes()) {
			const auto argTypeString = makeCType(argType, stream);
			stream << "typedef " << argTypeString << " ArgType" << argId << ";" << std::endl;
			argId++;
		}
		stream << std::endl;
	}
	
	{
		stream << "extern \"C\" ReturnType callee(";
		bool first = true;
		int argId = 0;
		for (const auto& argType: functionType.argumentTypes()) {
			if (first) {
				first = false;
			} else {
				stream << ", ";
			}
			stream << "ArgType" << argId << " arg" << argId;
			argId++;
		}
		if (functionType.argumentTypes().empty()) {
			stream << "void";
		}
		stream << ");" << std::endl << std::endl;
	}
	
	{
		stream << "extern \"C\" ReturnType caller(";
		bool first = true;
		int argId = 0;
		for (const auto& argType: functionType.argumentTypes()) {
			if (first) {
				first = false;
			} else {
				stream << ", ";
			}
			stream << "ArgType" << argId << " arg" << argId;
			argId++;
		}
		stream << ") {" << std::endl;
		
		stream << "     return callee(";
		first = true;
		argId = 0;
		for (const auto& argType: functionType.argumentTypes()) {
			if (first) {
				first = false;
			} else {
				stream << ", ";
			}
			stream << "arg" << argId;
			argId++;
		}
		stream << ");" << std::endl;
		stream << "}" << std::endl;
	}
	return stream.str();
}

std::string runClangOnFunction(const std::string& clangPath, const FunctionType& functionType) {
	if (clangPath.empty()) {
		printf("WARNING: No clang path provided!\n");
		return "";
	}
	
	std::ofstream tempFile("tempfile.cpp");
	tempFile << makeClangCCode(functionType);
	tempFile.close();
	
	const std::string cmd = clangPath + " -std=c++11 -S -emit-llvm tempfile.cpp -o tempfile.ll";
	
	const int result = system(cmd.c_str());
	if (result != EXIT_SUCCESS) {
		printf("Code was:\n%s\n", makeClangCCode(functionType).c_str());
		throw std::runtime_error("Clang failed!");
	}
	
	std::ifstream tempIRFile("tempfile.ll");
	return std::string(std::istreambuf_iterator<char>(tempIRFile),
	                   std::istreambuf_iterator<char>());
}

int main(int argc, char** argv) {
	assert(argc >= 2 && argc <= 3);
	
	const std::string string(argv[1]);
	const std::string clangPath = argc >= 3 ? argv[2] : "";
	printf("Clang: %s\n", clangPath.c_str());
	
	std::string abiString;
	std::string functionTypeString = "";
	
	std::ifstream file(string.c_str());
	
	const std::string ABI_COMMAND = "ABI";
	const std::string FUNCTION_TYPE_COMMAND = "FUNCTION-TYPE";
	
	std::vector<std::string> compareLines;
	
	std::string line;
	while (std::getline(file, line)) {
		if (line.empty()) {
			continue;
		}
		
		if (line[0] == ';') {
			size_t i = 1;
			while (i < line.size() && line[i] == ' ') {
				i++;
			}
			
			// This is a command.
			if (line.substr(i, ABI_COMMAND.size()) == ABI_COMMAND) {
				abiString = line.substr(i + ABI_COMMAND.size() + 2);
			} else if (line.substr(i, FUNCTION_TYPE_COMMAND.size()) == FUNCTION_TYPE_COMMAND) {
				functionTypeString = line.substr(i + FUNCTION_TYPE_COMMAND.size() + 1);
			}
		} else {
			compareLines.push_back(line);
		}
	}
	
	llvm_abi::TokenStream stream(functionTypeString);
	llvm_abi::TypeParser parser(stream);
	
	const auto functionType = parser.parseFunctionType();
	
	TestSystem testSystem(abiString);
	
	printf("Running test for function type: %s\n", functionType.toString().c_str());
	
	const auto fileName = getBaseName(getFileName(string));
	printf("filename = %s\n", fileName.c_str());
	
	testSystem.doTest(fileName, functionType);
	{
		std::string filename;
		filename += "test-";
		filename += "x86_64";
		filename += "-" + fileName;
		filename += ".output.ll";
		
		std::ifstream outputFile(filename);
		
		size_t nextLine = 0;
		
		while (std::getline(outputFile, line)) {
			if (line.empty() || line[0] == ';') {
				continue;
			}
			
			assert(nextLine < compareLines.size());
			
			if (compareLines[nextLine] != line) {
				printf("Lines not equal:\n  %s\n  %s\n\n\n",
				       compareLines[nextLine].c_str(), line.c_str());
				printf("---- Expected output:\n");
				std::ifstream expectedFile(string.c_str());
				while (std::getline(expectedFile, line)) {
					printf("%s\n", line.c_str());
				}
				printf("\n---- Actual output:\n");
				outputFile.seekg(0, outputFile.beg);
				while (std::getline(outputFile, line)) {
					printf("%s\n", line.c_str());
				}
				
				const auto cCompilerOutput = runClangOnFunction(clangPath, functionType);
				printf("\n---- C compiler output (%s):\n%s\n\n",
				       clangPath.c_str(),
				       cCompilerOutput.c_str());
				return EXIT_FAILURE;
			}
			
			nextLine++;
		}
	}
	
	printf("Test PASSED.\n");
	
	return 0;
}
