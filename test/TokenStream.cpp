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
