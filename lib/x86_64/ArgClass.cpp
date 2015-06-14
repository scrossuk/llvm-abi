#include "ArgClass.hpp"

namespace llvm_abi {
	
	namespace x86_64 {
		
		// Class merge operation as specified in ABI.
		ArgClass mergeClasses(ArgClass first, ArgClass second) {
			// AMD64-ABI 3.2.3p2: Rule 4. Each field of an object is
			// classified recursively so that always two fields are
			// considered. The resulting class is calculated according to
			// the classes of the fields in the eightbyte:
			
			// (a) If both classes are equal, this is the resulting class.
			if (first == second) {
				return first;
			}
			
			// (b) If one of the classes is NO_CLASS, the resulting class is
			// the other class.
			if (first == NoClass) {
				return second;
			}
			
			if (second == NoClass) {
				return first;
			}
			
			// (c) If one of the classes is MEMORY, the result is the MEMORY
			// class.
			if (first == Memory || second == Memory) {
				return Memory;
			}
			
			// (d) If one of the classes is INTEGER, the result is the
			// INTEGER.
			if (first == Integer || second == Integer) {
				return Integer;
			}
			
			// (e) If one of the classes is X87, X87UP, COMPLEX_X87 class,
			// MEMORY is used as class.
			if (first == X87 || first == X87Up || first == ComplexX87 ||
				second == X87 || second == X87Up || second == ComplexX87) {
				return Memory;
			}
			
			// (f) Otherwise class SSE is used.
			return Sse;
		}
		
	}
	
}

