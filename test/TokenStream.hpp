#ifndef TOKENSTREAM_HPP
#define TOKENSTREAM_HPP

#include <initializer_list>
#include <string>

namespace llvm_abi {
	
	class TokenStream {
	public:
		TokenStream(const std::string& text);
		
		char peek() const;
		
		void expect(char expectedToken) const;
		
		void expectAny(std::initializer_list<char> expectedTokens) const;
		
		void consume();
		
		void consumeOne();
		
		void consumeWhitespace();
		
	private:
		const std::string& text_;
		size_t offset_;
		
	};
	
}

#endif