#ifndef TYPEPARSER_HPP
#define TYPEPARSER_HPP

#include <string>

#include <llvm/ADT/SmallVector.h>

#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include "TestFunctionType.hpp"
#include "TokenStream.hpp"

namespace llvm_abi {

	class TypeParser {
	public:
		TypeParser(TokenStream& stream)
		: stream_(stream) { }
		
		std::string parseString() {
			std::string text;
			while (true) {
				const auto next = stream_.peek();
				if ((next >= 'a' && next <= 'z') ||
				    (next >= '0' && next <= '9')) {
					text += next;
					stream_.consume();
				} else {
					break;
				}
			}
			
			return text;
		}
		
		Type parseNamedType() {
			const auto text = parseString();
			assert(!text.empty());
			
			if (text == "void") {
				return VoidTy;
			} else if (text == "ptr") {
				return PointerTy;
			} else if (text == "bool") {
				return BoolTy;
			} else if (text == "char") {
				return CharTy;
			} else if (text == "schar") {
				return SCharTy;
			} else if (text == "uchar") {
				return UCharTy;
			} else if (text == "short") {
				return ShortTy;
			} else if (text == "ushort") {
				return UShortTy;
			} else if (text == "int") {
				return IntTy;
			} else if (text == "uint") {
				return UIntTy;
			} else if (text == "long") {
				return LongTy;
			} else if (text == "ulong") {
				return ULongTy;
			} else if (text == "longlong") {
				return LongLongTy;
			} else if (text == "ulonglong") {
				return ULongLongTy;
			} else if (text == "float") {
				return FloatTy;
			} else if (text == "double") {
				return DoubleTy;
			} else if (text == "longdouble") {
				return LongDoubleTy;
			} else {
				throw std::runtime_error(std::string("Unknown type '") + text + "'.");
			}
		}
		
		Type parseStructType() {
			assert(stream_.peek() == '{');
			stream_.consume();
			
			llvm::SmallVector<Type, 8> types;
			
			while (stream_.peek() != '}') {
				types.push_back(parseType());
				assert(stream_.peek() == ',' || stream_.peek() == '}');
				if (stream_.peek() == ',') {
					stream_.consume();
				}
			}
			
			assert(stream_.peek() == '}');
			stream_.consume();
			
			return typeBuilder_.getStructTy(types);
		}
		
		std::string parseIntString() {
			std::string text;
			while (true) {
				const auto next = stream_.peek();
				if (next >= '0' && next <= '9') {
					text += next;
					stream_.consume();
				} else {
					break;
				}
			}
			
			assert(!text.empty());
			
			return text;
		}
		
		int parseInt() {
			return atoi(parseIntString().c_str());
		}
		
		Type parseVectorType() {
			assert(stream_.peek() == '<');
			stream_.consume();
			
			const auto numElements = parseInt();
			
			assert(stream_.peek() == 'x');
			stream_.consume();
			
			const auto elementType = parseType();
			
			assert(stream_.peek() == '>');
			stream_.consume();
			
			return typeBuilder_.getVectorTy(numElements, elementType);
		}
		
		Type parseArrayType() {
			assert(stream_.peek() == '[');
			stream_.consume();
			
			const auto numElements = parseInt();
			
			assert(stream_.peek() == 'x');
			stream_.consume();
			
			const auto elementType = parseType();
			
			assert(stream_.peek() == ']');
			stream_.consume();
			
			return typeBuilder_.getArrayTy(numElements, elementType);
		}
		
		Type parseType() {
			const auto next = stream_.peek();
			
			if (next == '{') {
				// Struct type.
				return parseStructType();
			} else if (next == '<') {
				// Vector type.
				return parseVectorType();
			} else if (next == '[') {
				// Array type.
				return parseArrayType();
			} else if (next >= 'a' && next <= 'z') {
				// Named type.
				return parseNamedType();
			} else {
				throw std::runtime_error("Invalid type string.");
			}
		}
		
		llvm::SmallVector<Type, 8> parseVarArgsTypes() {
			assert(stream_.peek() == '.');
			stream_.consume();
			assert(stream_.peek() == '.');
			stream_.consume();
			assert(stream_.peek() == '.');
			stream_.consume();
			
			assert(stream_.peek() == '(');
			stream_.consume();
			
			llvm::SmallVector<Type, 8> varArgsTypes;
			
			while (stream_.peek() != ')') {
				varArgsTypes.push_back(parseType());
				assert(stream_.peek() == ',' || stream_.peek() == ')');
				if (stream_.peek() == ',') {
					stream_.consume();
				}
			}
			
			assert(stream_.peek() == ')');
			stream_.consume();
			
			return varArgsTypes;
		}
		
		TestFunctionType parseFunctionType() {
			const auto returnType = parseType();
			
			assert(stream_.peek() == '(');
			stream_.consume();
			
			llvm::SmallVector<Type, 8> argumentTypes;
			
			while (stream_.peek() != ')' && stream_.peek() != '.') {
				argumentTypes.push_back(parseType());
				assert(stream_.peek() == ',' || stream_.peek() == ')');
				if (stream_.peek() == ',') {
					stream_.consume();
				}
			}
			
			llvm::SmallVector<Type, 8> varArgsTypes;
			const bool isVarArg = (stream_.peek() == '.');
			if (isVarArg) {
				varArgsTypes = parseVarArgsTypes();
			}
			
			assert(stream_.peek() == ')');
			stream_.consume();
			
			return TestFunctionType(FunctionType(returnType,
			                                     argumentTypes,
			                                     isVarArg),
			                        varArgsTypes);
		}
		
	private:
		TokenStream& stream_;
		TypeBuilder typeBuilder_;
		
	};
	
}

#endif