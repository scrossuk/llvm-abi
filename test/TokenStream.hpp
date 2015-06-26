#ifndef TOKENSTREAM_HPP
#define TOKENSTREAM_HPP

#include <string>

namespace llvm_abi {
	
	class TokenStream {
	public:
		TokenStream(const std::string& text)
		: text_(text), offset_(0) { }
		
		char peek() {
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
		
		void consume() {
			offset_++;
		}
		
	private:
		const std::string& text_;
		size_t offset_;
		
	};
	
}

#endif