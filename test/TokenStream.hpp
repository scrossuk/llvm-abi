#ifndef TOKENSTREAM_HPP
#define TOKENSTREAM_HPP

#include <string>

namespace llvm_abi {
	
	class TokenStream {
	public:
		TokenStream(const std::string& text);
		
		char peek();
		
		void consume();
		
	private:
		const std::string& text_;
		size_t offset_;
		
	};
	
}

#endif