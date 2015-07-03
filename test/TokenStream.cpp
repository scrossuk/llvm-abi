#include <string>

#include "TokenStream.hpp"

namespace llvm_abi {
	
	TokenStream::TokenStream(const std::string& text)
	: text_(text), offset_(0) { }
	
	char TokenStream::peek() {
		while (true) {
			if (offset_ >= text_.length()) {
				return '\0';
			}
			
			if (text_[offset_] == ' ') {
				// Ignore spaces.
				offset_++;
				continue;
			}
			
			return text_[offset_];
		}
	}
	
	void TokenStream::consume() {
		offset_++;
	}
	
}
