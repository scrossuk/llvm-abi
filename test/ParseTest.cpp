#include <fstream>
#include <sstream>

#include "CCodeGenerator.hpp"
#include "TestFunctionType.hpp"
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

std::string runClangOnFunction(const ABITypeInfo& typeInfo,
                               const std::string& abiString,
                               const std::string& cpuString,
                               const std::string& clangPath,
                               const TestFunctionType& testFunctionType) {
	if (clangPath.empty()) {
		printf("WARNING: No clang path provided!\n");
		return "";
	}
	
	CCodeGenerator cCodeGenerator(typeInfo);
	cCodeGenerator.emitCalleeAndCallerFunctions(testFunctionType);
	
	std::ofstream tempFile("tempfile.c");
	tempFile << cCodeGenerator.generatedSourceCode();
	tempFile.close();
	
	std::string cmd = clangPath + " -target " + abiString + " ";
	if (!cpuString.empty()) {
		cmd += "-march=" + cpuString + " ";
	}
	cmd += "-S -emit-llvm tempfile.c -o tempfile.ll";
	
	const int result = system(cmd.c_str());
	if (result != EXIT_SUCCESS) {
		printf("Code was:\n%s\n", cCodeGenerator.generatedSourceCode().c_str());
		throw std::runtime_error("Clang failed!");
	}
	
	std::ifstream tempIRFile("tempfile.ll");
	return std::string(std::istreambuf_iterator<char>(tempIRFile),
	                   std::istreambuf_iterator<char>());
}

bool startsWith(const std::string& string, const std::string& substring,
                size_t position = 0) {
	return string.substr(position, substring.size()) == substring;
}

class Lexer {
public:
	Lexer(const std::string& line)
	: line_(line), pos_(0) { }
	
	char get() const {
		if (pos_ < line_.length()) {
			return line_[pos_];
		} else {
			return 0;
		}
	}
	
	bool isSpace() const {
		return get() == ' ';
	}
	
	bool isDigit() const {
		return '0' <= get() && get() <= '9';
	}
	
	bool isLowerAlpha() const {
		return 'a' <= get() && get() <= 'z';
	}
	
	bool isUpperAlpha() const {
		return 'A' <= get() && get() <= 'Z';
	}
	
	bool isAlpha() const {
		return isLowerAlpha() || isUpperAlpha();
	}
	
	bool isDigitOrAlpha() const {
		return isDigit() || isAlpha();
	}
	
	void advance() {
		assert(pos_ < line_.length());
		pos_++;
	}
	
	char expect(const char expected) {
		if (get() != expected) {
			throw std::runtime_error("Failed to get expected character.");
		}
		advance();
		return expected;
	}
	
	const std::string& expectString(const std::string& expected) {
		for (const char c: expected) {
			(void) expect(c);
		}
		return expected;
	}
	
	char expectSpace() {
		if (!isSpace()) {
			throw std::runtime_error("Failed to get space.");
		}
		const char c = get();
		advance();
		return c;
	}
	
	void consumeZeroOrMoreSpaces() {
		while (isSpace()) {
			advance();
		}
	}
	
	std::string expectOneOrMoreSpaces() {
		std::string text;
		text += expectSpace();
		
		while (isSpace()) {
			text += get();
			advance();
		}
		
		return text;
	}
	
	char expectAlpha() {
		if (!isAlpha()) {
			throw std::runtime_error("Failed to get alpha.");
		}
		const char c = get();
		advance();
		return c;
	}
	
	std::string expectOneOrMoreAlphas() {
		std::string text;
		text += expectAlpha();
		
		while (isAlpha()) {
			text += get();
			advance();
		}
		
		return text;
	}
	
	char expectDigit() {
		if (!isDigit()) {
			throw std::runtime_error("Failed to get digit.");
		}
		const char c = get();
		advance();
		return c;
	}
	
	std::string expectOneOrMoreDigits() {
		std::string text;
		text += expectDigit();
		
		while (isDigit()) {
			text += get();
			advance();
		}
		
		return text;
	}
	
	char expectDigitOrAlpha() {
		if (!isDigitOrAlpha()) {
			throw std::runtime_error("Failed to get digit or alpha.");
		}
		const char c = get();
		advance();
		return c;
	}
	
	std::string expectOneOrMoreDigitOrAlphas() {
		std::string text;
		text += expectDigitOrAlpha();
		
		while (isDigitOrAlpha()) {
			text += get();
			advance();
		}
		
		return text;
	}
	
private:
	const std::string& line_;
	size_t pos_;
	
};

class TypeInfo {
public:
	enum Kind {
		NAME,
		POINTER,
		ARRAY,
		VECTOR,
		STRUCT
	};
	
	static TypeInfo Name(std::string argName) {
		TypeInfo typeInfo(NAME);
		typeInfo.name_ = std::move(argName);
		return typeInfo;
	}
	
	static TypeInfo Pointer(TypeInfo type) {
		TypeInfo typeInfo(POINTER);
		typeInfo.typePtr_ = std::unique_ptr<TypeInfo>(new TypeInfo(std::move(type)));
		return typeInfo;
	}
	
	static TypeInfo Array(std::string argCount, TypeInfo type) {
		TypeInfo typeInfo(ARRAY);
		typeInfo.name_ = std::move(argCount);
		typeInfo.typePtr_ = std::unique_ptr<TypeInfo>(new TypeInfo(std::move(type)));
		return typeInfo;
	}
	
	static TypeInfo Vector(std::string argCount, TypeInfo type) {
		TypeInfo typeInfo(VECTOR);
		typeInfo.name_ = std::move(argCount);
		typeInfo.typePtr_ = std::unique_ptr<TypeInfo>(new TypeInfo(std::move(type)));
		return typeInfo;
	}
	
	static TypeInfo Struct(std::vector<TypeInfo> types) {
		TypeInfo typeInfo(STRUCT);
		typeInfo.typeArray_ = std::move(types);
		return typeInfo;
	}
	
	TypeInfo(TypeInfo&&) = default;
	TypeInfo& operator=(TypeInfo&&) = default;
	
	Kind kind() const {
		return kind_;
	}
	
	bool isName() const {
		return kind() == NAME;
	}
	
	bool isPointer() const {
		return kind() == POINTER;
	}
	
	bool isArray() const {
		return kind() == ARRAY;
	}
	
	bool isVector() const {
		return kind() == VECTOR;
	}
	
	bool isStruct() const {
		return kind() == STRUCT;
	}
	
	const std::string& name() const {
		assert(isName());
		return name_;
	}
	
	const TypeInfo& pointerElementType() const {
		assert(isPointer());
		return *typePtr_;
	}
	
	const std::string& arrayCount() const {
		assert(isArray());
		return name_;
	}
	
	const TypeInfo& arrayElementType() const {
		assert(isArray());
		return *typePtr_;
	}
	
	const std::string& vectorCount() const {
		assert(isVector());
		return name_;
	}
	
	const TypeInfo& vectorElementType() const {
		assert(isVector());
		return *typePtr_;
	}
	
	const std::vector<TypeInfo>& structElementTypes() const {
		assert(isStruct());
		return typeArray_;
	}
	
	TypeInfo copy() const {
		switch (kind()) {
			case NAME:
				return TypeInfo::Name(name());
			case POINTER:
				return TypeInfo::Pointer(pointerElementType().copy());
			case ARRAY:
				return TypeInfo::Array(arrayCount(), arrayElementType().copy());
			case VECTOR:
				return TypeInfo::Vector(vectorCount(), vectorElementType().copy());
			case STRUCT: {
				std::vector<TypeInfo> typeCopies;
				for (const auto& structType: structElementTypes()) {
					typeCopies.push_back(structType.copy());
				}
				return TypeInfo::Struct(std::move(typeCopies));
			}
		}
		
		throw std::logic_error("Unknown type kind.");
	}
	
	std::string toString() const {
		std::stringstream stream;
		switch (kind()) {
		case NAME:
			stream << name();
			break;
		case POINTER:
			stream << pointerElementType().toString() << "*";
			break;
		case ARRAY:
			stream << "[";
			stream << arrayCount();
			stream << " x ";
			stream << arrayElementType().toString();
			stream << "]";
			break;
		case VECTOR:
			stream << "<";
			stream << vectorCount();
			stream << " x ";
			stream << vectorElementType().toString();
			stream << ">";
			break;
		case STRUCT:
			stream << "{ ";
			bool isFirst = true;
			for (const auto& structType: structElementTypes()) {
				if (isFirst) {
					isFirst = false;
				} else {
					stream << ", ";
				}
				stream << structType.toString();
			}
			stream << " }";
			break;
		}
		return stream.str();
	}
	
private:
	TypeInfo(const Kind argKind)
	: kind_(argKind) { }
	
	Kind kind_;
	std::string name_;
	std::unique_ptr<TypeInfo> typePtr_;
	std::vector<TypeInfo> typeArray_;
	
};

class GEPInfo {
public:
	GEPInfo(std::string argResultName, TypeInfo argOpType,
	        TypeInfo argPtrType, std::string argPtrName)
	: resultName_(std::move(argResultName)),
	  opType_(std::move(argOpType)),
	  ptrType_(std::move(argPtrType)),
	  ptrName_(std::move(argPtrName)) { }
	
	std::string toString() const {
		std::stringstream stream;
		stream << "GEP(";
		stream << "resultName: " << resultName_ << ", ";
		stream << "opType: " << opType_.toString() << ", ";
		stream << "ptrType: " << ptrType_.toString() << ", ";
		stream << "ptrName: " << ptrName_;
		stream << ")";
		return stream.str();
	}
	
private:
	std::string resultName_;
	TypeInfo opType_;
	TypeInfo ptrType_;
	std::string ptrName_;
	
};

class Parser {
public:
	Parser(Lexer& lexer)
	: lexer_(lexer) { }
	
	std::string parseNameString() {
		std::string text;
		text += lexer_.expectDigitOrAlpha();
		while (true) {
			if (lexer_.isDigitOrAlpha()) {
				text += lexer_.expectDigitOrAlpha();
			} else if (lexer_.get() == '.') {
				text += lexer_.expect('.');
			} else if (lexer_.get() == '_') {
				text += lexer_.expect('_');
			} else {
				break;
			}
		}
		return text;
	}
	
	std::string parseVar() {
		std::string text;
		text += lexer_.expect('%');
		text += parseNameString();
		return text;
	}
	
	std::string parseFunctionName() {
		std::string text;
		text += lexer_.expect('@');
		text += parseNameString();
		return text;
	}
	
	std::string parseValue() {
		if (lexer_.get() == '%') {
			return parseVar();
		} else {
			return parseFunctionName();
		}
	}
	
	TypeInfo parseStructType() {
		lexer_.expect('{');
		lexer_.consumeZeroOrMoreSpaces();
		
		if (lexer_.get() == '}') {
			lexer_.expect('}');
			return TypeInfo::Struct({});
		}
		
		std::vector<TypeInfo> types;
		
		while (true) {
			types.push_back(parseType());
			lexer_.consumeZeroOrMoreSpaces();
			if (lexer_.get() == '}') {
				break;
			} else {
				lexer_.expect(',');
				lexer_.consumeZeroOrMoreSpaces();
			}
		}
		
		lexer_.expect('}');
		return TypeInfo::Struct(std::move(types));
	}
	
	TypeInfo parseArrayType() {
		lexer_.expect('[');
		lexer_.consumeZeroOrMoreSpaces();
		
		const auto count = lexer_.expectOneOrMoreDigits();
		lexer_.consumeZeroOrMoreSpaces();
		
		lexer_.expect('x');
		lexer_.expectOneOrMoreSpaces();
		
		auto type = parseType();
		lexer_.consumeZeroOrMoreSpaces();
		
		lexer_.expect(']');
		return TypeInfo::Array(count, std::move(type));
	}
	
	TypeInfo parseVectorType() {
		lexer_.expect('<');
		lexer_.consumeZeroOrMoreSpaces();
		
		const auto count = lexer_.expectOneOrMoreDigits();
		lexer_.consumeZeroOrMoreSpaces();
		
		lexer_.expect('x');
		lexer_.expectOneOrMoreSpaces();
		
		auto type = parseType();
		lexer_.consumeZeroOrMoreSpaces();
		
		lexer_.expect('>');
		return TypeInfo::Vector(count, std::move(type));
	}
	
	TypeInfo parseNonPointerType() {
		if (lexer_.get() == '{') {
			return parseStructType();
		} else if (lexer_.get() == '<') {
			return parseVectorType();
		} else if (lexer_.get() == '[') {
			return parseArrayType();
		} else {
			return TypeInfo::Name(parseNameString());
		}
	}
	
	TypeInfo parseType() {
		TypeInfo type = parseNonPointerType();
		
		while (true) {
			lexer_.consumeZeroOrMoreSpaces();
			if (lexer_.get() == '*') {
				type = TypeInfo::Pointer(std::move(type));
				lexer_.advance();
			} else {
				break;
			}
		}
		
		return type;
	}
	
	std::string parseInstruction() {
		lexer_.consumeZeroOrMoreSpaces();
		
		if (lexer_.get() == '%') {
			const auto resultName = parseVar();
			lexer_.expectOneOrMoreSpaces();
			lexer_.expect('=');
			lexer_.expectOneOrMoreSpaces();
			
			const auto command = lexer_.expectOneOrMoreAlphas();
			lexer_.expectOneOrMoreSpaces();
			
			if (command == "getelementptr") {
				return parseGEP(resultName);
			} else if (command == "load") {
				return parseLoad(resultName);
			} else {
				printf("Result: %s; command is %s\n", resultName.c_str(), command.c_str());
				throw std::runtime_error("Unknown command.");
			}
		} else {
			const auto command = lexer_.expectOneOrMoreAlphas();
			lexer_.consumeZeroOrMoreSpaces();
			
			if (command == "call") {
				return parseCall("");
			} else {
				printf("Command is %s\n", command.c_str());
				throw std::runtime_error("Unknown command.");
			}
		}
	}
	
	std::string parseCall(const std::string& resultName) {
		auto returnType = parseType();
		std::vector<TypeInfo> argTypes;
		bool isVarArg = false;
		lexer_.consumeZeroOrMoreSpaces();
		lexer_.expect('(');
		lexer_.consumeZeroOrMoreSpaces();
		while (true) {
			if (lexer_.get() == '.') {
				lexer_.expectString("...");
				lexer_.consumeZeroOrMoreSpaces();
				isVarArg = true;
				lexer_.expect(')');
				lexer_.consumeZeroOrMoreSpaces();
				break;
			} else if (lexer_.get() == ')') {
				lexer_.expect(')');
				lexer_.consumeZeroOrMoreSpaces();
				break;
			} else {
				argTypes.push_back(parseType());
				lexer_.consumeZeroOrMoreSpaces();
				if (lexer_.get() != ')') {
					lexer_.expect(',');
					lexer_.consumeZeroOrMoreSpaces();
				}
			}
		}
		
		if (lexer_.get() == '*') {
			lexer_.expect('*');
			lexer_.consumeZeroOrMoreSpaces();
		}
		
		const auto callValue = parseValue();
		lexer_.consumeZeroOrMoreSpaces();
		
		lexer_.expect('(');
		lexer_.consumeZeroOrMoreSpaces();
		
		std::vector<TypeInfo> argValueTypes;
		std::vector<std::string> argValues;
		
		while (lexer_.get() != ')') {
			argValueTypes.push_back(parseType());
			lexer_.consumeZeroOrMoreSpaces();
			
			std::string value;
			
			while (lexer_.isDigitOrAlpha()) {
				const auto property = lexer_.expectOneOrMoreDigitOrAlphas();
				lexer_.consumeZeroOrMoreSpaces();
				value += property;
				value += " ";
			}
			
			value += parseValue();
			
			argValues.push_back(value);
			lexer_.consumeZeroOrMoreSpaces();
			
			if (lexer_.get() != ')') {
				lexer_.expect(',');
				lexer_.consumeZeroOrMoreSpaces();
			}
		}
		
		lexer_.expect(')');
		
		std::stringstream stream;
		
		stream << "Call(";
		stream << "returnType: " << returnType.toString() << ", ";
		stream << "argTypes: [";
		for (const auto& argType: argTypes) {
			stream << argType.toString() << ", ";
		}
		stream << "], ";
		stream << "isVarArg: " << (isVarArg ? "true" : "false") << ", ";
		stream << "callValue: " << callValue << ", ";
		stream << "argValueTypes: [";
		for (const auto& argType: argValueTypes) {
			stream << argType.toString() << ", ";
		}
		stream << "], ";
		stream << "argValues: [";
		for (const auto& argValue: argValues) {
			stream << argValue << ", ";
		}
		stream << "])";
		return stream.str();
	}
	
	std::string parseLoad(const std::string& resultName) {
		auto type = parseType();
		lexer_.consumeZeroOrMoreSpaces();
		
		auto opType = TypeInfo::Name("");
		auto ptrType = TypeInfo::Name("");
		
		if (lexer_.get() == ',') {
			// New style getelementptr with operation type.
			lexer_.advance();
			
			lexer_.consumeZeroOrMoreSpaces();
			
			opType = std::move(type);
			ptrType = parseType();
			
			lexer_.consumeZeroOrMoreSpaces();
		} else {
			assert(type.isPointer());
			opType = type.pointerElementType().copy();
			ptrType = std::move(type);
		}
		
		const auto ptrName = parseVar();
		lexer_.consumeZeroOrMoreSpaces();
		
		std::stringstream stream;
		stream << "Load(";
		stream << "resultName: " << resultName << ", ";
		stream << "opType: " << opType.toString() << ", ";
		stream << "ptrType: " << ptrType.toString() << ", ";
		stream << "ptrName: " << ptrName;
		stream << ")";
		return stream.str();
	}
	
	std::string parseGEP(const std::string& resultName) {
		auto type = parseType();
		lexer_.consumeZeroOrMoreSpaces();
		
		auto opType = TypeInfo::Name("");
		auto ptrType = TypeInfo::Name("");
		
		if (lexer_.get() == ',') {
			// New style getelementptr with operation type.
			lexer_.advance();
			
			lexer_.consumeZeroOrMoreSpaces();
			
			opType = std::move(type);
			ptrType = parseType();
			
			lexer_.consumeZeroOrMoreSpaces();
		} else {
			assert(type.isPointer());
			opType = type.pointerElementType().copy();
			ptrType = std::move(type);
		}
		
		const auto ptrName = parseVar();
		lexer_.consumeZeroOrMoreSpaces();
		lexer_.expect(',');
		
		// Ignore the rest...
		GEPInfo gepInfo(resultName, std::move(opType),
		                std::move(ptrType), ptrName);
		return gepInfo.toString();
	}
	
private:
	Lexer& lexer_;
	
};

std::string getCanonicalInstruction(const std::string& instruction) {
	Lexer lexer(instruction);
	Parser parser(lexer);
	return parser.parseInstruction();
}

static const std::string MEMCPY_START = "declare void @llvm.memcpy";

bool linesAreEqual(const std::string& expected, const std::string& actual) {
	if (expected == actual) {
		return true;
	}
	
	if (startsWith(expected, MEMCPY_START) &&
	    startsWith(actual, MEMCPY_START)) {
		// The memcpy intrinsic has different attributes
		// for different versions of LLVM, but we don't
		// care about these differences.
		return true;
	}
	
	try {
		// Some instructions vary between LLVM versions (e.g. 'load' has
		// an extra type parameter in LLVM 3.7+ as opaque pointers are
		// added to LLVM). In these cases we parse the IR and generate
		// a 'canonical' string, which allows us to check whether these
		// instructions are equivalent without hitting LLVM
		// version-specific variations.
		const auto expectedCanonical = getCanonicalInstruction(expected);
		const auto actualCanonical = getCanonicalInstruction(actual);
		if (expectedCanonical == actualCanonical) {
			return true;
		} else {
			printf("%s vs %s\n", expectedCanonical.c_str(),
			       actualCanonical.c_str());
		}
	} catch (const std::exception&) {
		printf("Parsing failed for: %s\n", actual.c_str());
	}
	
	return false;
}

int main(int argc, char** argv) {
	assert(argc >= 2 && argc <= 3);
	
	const std::string string(argv[1]);
	const std::string clangPath = argc >= 3 ? argv[2] : "";
	printf("Clang: %s\n", clangPath.c_str());
	
	std::string abiString;
	std::string cpuString;
	std::string functionTypeString = "";
	
	std::ifstream file(string.c_str());
	
	const std::string ABI_COMMAND = "ABI";
	const std::string CPU_COMMAND = "CPU";
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
			} else if (line.substr(i, CPU_COMMAND.size()) == CPU_COMMAND) {
				cpuString = line.substr(i + CPU_COMMAND.size() + 2);
			} else if (line.substr(i, FUNCTION_TYPE_COMMAND.size()) == FUNCTION_TYPE_COMMAND) {
				functionTypeString = line.substr(i + FUNCTION_TYPE_COMMAND.size() + 1);
			}
		} else {
			compareLines.push_back(line);
		}
	}
	
	if (abiString.empty()) {
		printf("ERROR: No ABI specified.\n");
		return EXIT_FAILURE;
	}
	
	llvm_abi::TokenStream stream(functionTypeString);
	llvm_abi::TypeParser parser(stream);
	
	const auto testFunctionType = parser.parseFunctionType();
	
	TestSystem testSystem(abiString, cpuString);
	
	printf("Running test for function type: %s\n", testFunctionType.functionType.toString().c_str());
	
	const auto fileName = getBaseName(getFileName(string));
	printf("filename = %s\n", fileName.c_str());
	
	testSystem.doTest(fileName, testFunctionType);
	
	{
		std::string filename;
		filename += "test-";
		filename += testSystem.abi().name();
		filename += "-" + fileName;
		filename += ".output.ll";
		
		std::ifstream outputFile(filename);
		if (!outputFile.is_open()) {
			printf("Failed to open output file!\n");
			return EXIT_FAILURE;
		}
		
		size_t nextLine = 0;
		
		while (std::getline(outputFile, line)) {
			if (line.empty() || line[0] == ';') {
				continue;
			}
			
			if (nextLine < compareLines.size() &&
			    startsWith(line, MEMCPY_START) &&
			    startsWith(compareLines[nextLine], MEMCPY_START)) {
				// The memcpy intrinsic has different attributes
				// for different versions of LLVM, but we don't
				// care about these differences.
				nextLine++;
				continue;
			}
			
			if (nextLine >= compareLines.size() ||
			    !linesAreEqual(compareLines[nextLine], line)) {
				if (nextLine >= compareLines.size()) {
					printf("Actual output was too long...\n\n");
				} else if (compareLines[nextLine] != line) {
					printf("Lines not equal:\n  %s\n  %s\n\n\n",
					       compareLines[nextLine].c_str(), line.c_str());
				}
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
				
				const auto cCompilerOutput =
					runClangOnFunction(testSystem.abi().typeInfo(),
					                   abiString,
					                   cpuString,
					                   clangPath,
					                   testFunctionType);
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
