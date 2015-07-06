#ifndef LLVMABI_FUNCTIONIRMAPPING_HPP
#define LLVMABI_FUNCTIONIRMAPPING_HPP

#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/ArgInfo.hpp>
#include <llvm-abi/ArgumentIRMapping.hpp>
#include <llvm-abi/FunctionType.hpp>

namespace llvm_abi {
	
	/**
	 * \brief Mapping from ABI function to IR function.
	 * 
	 * This struct holds the information necessary to translate
	 * from an ABI function to an LLVM IR function, such as
	 * the ranges of LLVM IR arguments for each ABI arguments,
	 * whether there's a struct-return argument etc.
	 */
	class FunctionIRMapping {
	public:
		FunctionIRMapping()
		: inallocaArgIndex_(InvalidIndex),
		structRetArgIndex_(InvalidIndex),
		totalIRArgs_(0) { }
		
		void setReturnArgInfo(const ArgInfo& newReturnArgInfo) {
			returnArgInfo_ = newReturnArgInfo;
		}
		
		const ArgInfo& returnArgInfo() const {
			return returnArgInfo_;
		}
		
 		llvm::ArrayRef<ArgumentIRMapping> arguments() const {
 			return arguments_;
		}
		
 		llvm::SmallVector<ArgumentIRMapping, 8>& arguments() {
 			return arguments_;
		}
		
		/**
		 * \brief Query whether function has 'inalloca' argument.
		 */
		bool hasInallocaArg() const {
			return inallocaArgIndex_ != InvalidIndex;
		}
		
		/**
		 * \brief Get the index of the 'inalloca' argument.
		 */
		size_t inallocaArgIndex() const {
			assert(hasInallocaArg());
			return inallocaArgIndex_;
		}
		
		/**
		 * \brief Set the index of the 'inalloca' argument.
		 */
		void setInallocaArgIndex(const size_t argIndex) {
			inallocaArgIndex_ = argIndex;
		}
		
		/**
		 * \brief Query whether function has 'sret' argument.
		 */
		bool hasStructRetArg() const {
			return structRetArgIndex_ != InvalidIndex;
		}
		
		/**
		 * \brief Get the index of the 'sret' argument.
		 */
		size_t structRetArgIndex() const {
			assert(hasStructRetArg());
			return structRetArgIndex_;
		}
		
		/**
		 * \brief Set the index of the 'sret' argument.
		 */
		void setStructRetArgIndex(const size_t argIndex) {
			structRetArgIndex_ = argIndex;
		}
		
		/**
		 * \brief Query whether argument has padding IR argument.
		 */
		bool hasPaddingArg(const size_t argIndex) const {
			assert(argIndex < arguments_.size());
			return arguments_[argIndex].paddingArgIndex != InvalidIndex;
		}
		
		/**
		 * \brief Get the index of the padding IR argument for
		 *        an ABI argument.
		 */
		size_t paddingArgIndex(const size_t argIndex) const {
			assert(hasPaddingArg(argIndex));
			return arguments_[argIndex].paddingArgIndex;
		}
		
		/**
		 * \brief Get total number of IR arguments.
		 */
		size_t totalIRArgs() const {
			return totalIRArgs_;
		}
		
		/**
		 * \brief Set total number of IR arguments.
		 */
		void setTotalIRArgs(const size_t newTotalIRArgs) {
			totalIRArgs_ = newTotalIRArgs;
		}
		
		/**
		 * \brief Get IR argument range for an ABI argument.
		 * 
		 * \param Argument index
		 * \return Pair of index of first IR argument and the
		 *         number of IR arguments.
		 */
		std::pair<size_t, size_t>
		getIRArgRange(const size_t argIndex) const {
			assert(argIndex < arguments_.size());
			return std::make_pair(arguments_[argIndex].firstArgIndex,
			                      arguments_[argIndex].numberOfIRArgs);
		}
		
	private:
		ArgInfo returnArgInfo_;
		size_t inallocaArgIndex_;
		size_t structRetArgIndex_;
		size_t totalIRArgs_;
		llvm::SmallVector<ArgumentIRMapping, 8> arguments_;
		
	};
	
	/**
	 * \brief Get function IR mapping.
	 * 
	 * \param argInfoArray The ArgInfo values for each argument.
	 * \return The mapping from ABI arguments to IR arguments.
	 */
	FunctionIRMapping
	getFunctionIRMapping(llvm::ArrayRef<ArgInfo> argInfoArray);
	
	/**
	 * \brief Get LLVM function type.
	 * 
	 * \param context The LLVM context.
	 * \param typeInfo The ABI type information.
	 * \param functionType The ABI function type.
	 * \param functionIRMapping The ABI function IR mapping.
	 * \return The ABI-encoded LLVM function type.
	 */
	llvm::FunctionType *
	getFunctionType(llvm::LLVMContext& context,
	                const ABITypeInfo& typeInfo,
	                const FunctionType& functionType,
	                const FunctionIRMapping& functionIRMapping);
	
	/**
	 * \brief Get LLVM function attributes.
	 * 
	 * \param context The LLVM context.
	 * \param typeInfo The ABI type information.
	 * \param functionIRMapping The ABI function IR mapping.
	 * \param existingAttributes Any existing attributes (that may need to
	 *                           be removed).
	 * \return The ABI-encoded LLVM function type.
	 */
	llvm::AttributeSet
	getFunctionAttributes(llvm::LLVMContext& llvmContext,
	                      const ABITypeInfo& typeInfo,
	                      const FunctionIRMapping& functionIRMapping,
	                      const llvm::AttributeSet existingAttributes);
	
}

#endif
