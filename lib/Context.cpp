#include <set>

#include <llvm-abi/Context.hpp>
#include <llvm-abi/Type.hpp>

namespace llvm_abi {
	
	Context::Context() { }
	
	const Type* Context::getType(Type type) {
		auto result = types_.insert(type);
		// Not sure why the const cast is needed here...
		return &(*(result.first));
	}
	
}

