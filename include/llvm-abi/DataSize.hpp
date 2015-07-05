#ifndef LLVMABI_DATASIZE_HPP
#define LLVMABI_DATASIZE_HPP

#include <stdint.h>

namespace llvm_abi {
	
	class DataSize {
	public:
		static DataSize Zero() {
			return DataSize(0);
		}
		
		static DataSize Bits(const uint64_t value) {
			return DataSize(value);
		}
		
		static DataSize Bytes(const uint64_t value) {
			return DataSize(value * 8);
		}
		
		DataSize() = default;
		
		bool isPowerOf2Bytes() const {
			const auto value = asBytes();
			return value != 0 && (value & (value - 1)) == 0;
		}
		
		DataSize roundUpToPowerOf2Bits() const {
			auto value = asBits();
			
			// Subtract one in case we're already a power of 2.
			if (value > 0) {
				value--;
			}
			
			// Or together every bit below the highest set bit; we
			// can do this a logarithmic number of operations.
			value |= value >> 1;
			value |= value >> 2;
			value |= value >> 4;
			value |= value >> 8;
			value |= value >> 16;
			value |= value >> 32;
			
			// At this point we have a number like:
			//     00001111
			// (i.e. some sequence of zeroes followed by some
			//  sequence of ones).
			// So add one and this will get the next power of 2.
			value++;
			
			return DataSize::Bits(value);
		}
		
		DataSize roundUpToPowerOf2Bytes() const {
			// Just round up to the next highest number of bytes
			// and then any powers of 2 of bits above that will also
			// be values in bytes.
			return roundUpToAlign(DataSize::Bytes(1)).roundUpToPowerOf2Bits();
		}
		
		DataSize roundUpToAlign(const DataSize alignment) const {
			assert(alignment.isIntegerNumberOfBytes());
			assert(alignment.isPowerOf2Bytes());
			const auto result = DataSize::Bits(
				(asBits() + (alignment.asBits() - 1)) &
				(~(alignment.asBits() - 1))
			);
			
			// If the original value was an integer number of bytes,
			// this new value should also be (since the alignment
			// must be an integer number of bytes).
			assert(!isIntegerNumberOfBytes() || result.isIntegerNumberOfBytes());
			
			return result;
		}
		
		bool operator==(const DataSize& other) const {
			return asBits() == other.asBits();
		}
		
		bool operator!=(const DataSize& other) const {
			return asBits() != other.asBits();
		}
		
		bool operator<(const DataSize& other) const {
			return asBits() < other.asBits();
		}
		
		bool operator<=(const DataSize& other) const {
			return asBits() <= other.asBits();
		}
		
		bool operator>(const DataSize& other) const {
			return asBits() > other.asBits();
		}
		
		bool operator>=(const DataSize& other) const {
			return asBits() >= other.asBits();
		}
		
		DataSize operator+(const DataSize& other) const {
			return DataSize::Bits(asBits() + other.asBits());
		}
		
		DataSize operator-(const DataSize& other) const {
			assert(*this >= other);
			return DataSize::Bits(asBits() - other.asBits());
		}
		
		DataSize operator*(const uint64_t multiplier) const {
			return DataSize::Bits(asBits() * multiplier);
		}
		
		uint64_t operator/(const DataSize& other) const {
			return asBits() / other.asBits();
		}
		
		DataSize& operator+=(const DataSize& other) {
			sizeInBits_ += other.asBits();
			return *this;
		}
		
		uint64_t asBits() const {
			return sizeInBits_;
		}
		
		bool isIntegerNumberOfBytes() const {
			return (asBits() % 8) == 0;
		}
		
		uint64_t asBytes() const {
			assert(isIntegerNumberOfBytes());
			return sizeInBits_ / 8;
		}
		
	private:
		explicit DataSize(const uint64_t sizeInBits)
		: sizeInBits_(sizeInBits) { }
		
		uint64_t sizeInBits_;
		
	};
	
}

#endif
