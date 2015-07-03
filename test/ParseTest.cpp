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

std::string runClangOnFunction(const std::string& abiString,
                               const std::string& cpuString,
                               const std::string& clangPath,
                               const TestFunctionType& testFunctionType) {
	if (clangPath.empty()) {
		printf("WARNING: No clang path provided!\n");
		return "";
	}
	
	CCodeGenerator cCodeGenerator;
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
		filename += "x86_64";
		filename += "-" + fileName;
		filename += ".output.ll";
		
		std::ifstream outputFile(filename);
		
		size_t nextLine = 0;
		
		while (std::getline(outputFile, line)) {
			if (line.empty() || line[0] == ';') {
				continue;
			}
			
			if (nextLine >= compareLines.size() ||
			    compareLines[nextLine] != line) {
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
				
				const auto cCompilerOutput = runClangOnFunction(abiString, cpuString, clangPath, testFunctionType);
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
