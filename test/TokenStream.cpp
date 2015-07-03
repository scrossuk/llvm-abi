#include <initializer_list>
#include <stdexcept>
#include <string>

#include "TokenStream.hpp"

namespace llvm_abi {
	
	TokenStream::TokenStream(const std::string& text)
	: text_(text), offset_(0) {
		consumeWhitespace();
	}
	
	char TokenStream::peek() const {
		if (offset_ >= text_.length()) {
			return '\0';
		} else {
			return text_[offset_];
		}
	}
	
	void TokenStream::expect(const char expectedToken) const {
		if (peek() != expectedToken) {
			throw std::runtime_error(std::string("Didn't find expected token '") + std::string(1, expectedToken) + "'.");
		}
	}
	
	void TokenStream::expectAny(std::initializer_list<char> expectedTokens) const {
		for (const auto& expectedToken: expectedTokens) {
			if (peek() == expectedToken) {
				return;
			}
		}
		throw std::runtime_error("Couldn't find expected token in list.");
	}
	
	void TokenStream::consume() {
		offset_++;
		consumeWhitespace();
	}
	
	void TokenStream::consumeWhitespace() {
		while (true) {
			if (offset_ >= text_.length()) {
				break;
			}
			
			if (text_[offset_] == ' ') {
				// Ignore spaces.
				offset_++;
				continue;
			}
			
			break;
		}
	}
	
}
