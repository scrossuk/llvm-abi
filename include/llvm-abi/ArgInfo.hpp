#ifndef LLVMABI_ARGINFO_HPP
#define LLVMABI_ARGINFO_HPP

#include <sstream>

#include <llvm-abi/Type.hpp>

namespace llvm_abi {
	
	/**
	 * \brief Argument information
	 * 
	 * A data structure used by ABIs to represent how a type is passed to
	 * or returned from a function.
	 */
	class ArgInfo {
	public:
		/**
		 * \brief Argument Information Kind
		 * 
		 * These enum values specify how to pass values should be passed to or
		 * returned from functions in order to comply with ABIs.
		 */
		enum ArgInfoKind {
			/**
			 * \brief Pass argument directly.
			 */
			Direct,
			
			/**
			 * \brief Extend integer argument.
			 */
			ExtendInteger,
			
			/**
			 * \brief Pass argument via hidden pointer.
			 */
			Indirect,
			
			/**
			 * \brief Ignore argument.
			 */
			Ignore,
			
			/**
			 * \brief Expand aggregate type.
			 */
			Expand,
			
			/**
			 * \brief Pass argument using LLVM inalloca attribute.
			 */
			InAlloca
		};
		
	private:
		Type typeData; // isDirect() || isExtend() || isExpand()
		Type paddingType;
		union {
			unsigned directOffset; // isDirect() || isExtend()
			unsigned indirectAlign; // isIndirect()
			unsigned allocaFieldIndex; // isInAlloca()
		};
		ArgInfoKind kind;
		bool paddingInReg : 1;
		bool inAllocaSRet : 1; // isInAlloca()
		bool indirectByVal : 1; // isIndirect()
		bool indirectRealign : 1; // isIndirect()
		bool sRetAfterThis : 1; // isIndirect()
		bool inReg : 1; // isDirect() || isExtend() || isIndirect()
		bool canBeFlattened: 1; // isDirect()

		ArgInfo(ArgInfoKind K)
		: directOffset(0),
		kind(K),
		paddingInReg(false),
		inAllocaSRet(false),
		indirectByVal(false),
		indirectRealign(false),
		sRetAfterThis(false),
		inReg(false),
		canBeFlattened(false) {}
		
	public:
		ArgInfo()
		: typeData(VoidTy), paddingType(VoidTy), directOffset(0),
		kind(Direct), paddingInReg(false), inReg(false) {}
		
		static ArgInfo getDirect(Type type,
		                         unsigned offset = 0,
		                         Type padding = VoidTy,
		                         bool canBeFlattened = true) {
			auto argInfo = ArgInfo(Direct);
			argInfo.setCoerceToType(type);
			argInfo.setDirectOffset(offset);
			argInfo.setPaddingType(padding);
			argInfo.setCanBeFlattened(canBeFlattened);
			return argInfo;
		}
		
		static ArgInfo getDirectInReg(Type type) {
			auto argInfo = getDirect(type);
			argInfo.setInReg(true);
			return argInfo;
		}
		
		static ArgInfo getExtend(Type type) {
			auto argInfo = ArgInfo(ExtendInteger);
			argInfo.setCoerceToType(type);
			argInfo.setDirectOffset(0);
			return argInfo;
		}
		
		static ArgInfo getExtendInReg(Type type) {
			auto argInfo = getExtend(type);
			argInfo.setInReg(true);
			return argInfo;
		}
		
		static ArgInfo getIgnore() {
			return ArgInfo(Ignore);
		}
		
		static ArgInfo getIndirect(unsigned alignment,
		                           bool byVal = true,
		                           bool realign = false,
		                           Type padding = VoidTy) {
			auto argInfo = ArgInfo(Indirect);
			argInfo.setIndirectAlign(alignment);
			argInfo.setIndirectByVal(byVal);
			argInfo.setIndirectRealign(realign);
			argInfo.setSRetAfterThis(false);
			argInfo.setPaddingType(padding);
			return argInfo;
		}
		
		static ArgInfo getIndirectInReg(unsigned alignment,
		                                bool byVal = true,
		                                bool realign = false) {
			auto argInfo = getIndirect(alignment,
			                           byVal,
			                           realign);
			argInfo.setInReg(true);
			return argInfo;
		}
		
		static ArgInfo getInAlloca(unsigned fieldIndex) {
			auto argInfo = ArgInfo(InAlloca);
			argInfo.setInAllocaFieldIndex(fieldIndex);
			return argInfo;
		}
		
		static ArgInfo getExpand(const Type expandType) {
			auto argInfo = ArgInfo(Expand);
			argInfo.setExpandType(expandType);
			return argInfo;
		}
		
		static ArgInfo getExpandWithPadding(const Type expandType,
		                                    bool paddingInReg,
		                                    Type padding) {
			auto argInfo = getExpand(expandType);
			argInfo.setPaddingInReg(paddingInReg);
			argInfo.setPaddingType(padding);
			return argInfo;
		}
		
		ArgInfoKind getKind() const { return kind; }
		bool isDirect() const { return kind == Direct; }
		bool isInAlloca() const { return kind == InAlloca; }
		bool isExtend() const { return kind == ExtendInteger; }
		bool isIgnore() const { return kind == Ignore; }
		bool isIndirect() const { return kind == Indirect; }
		bool isExpand() const { return kind == Expand; }
		
		bool canHaveCoerceToType() const { return isDirect() || isExtend(); }
		
		// Direct/Extend accessors
		unsigned getDirectOffset() const {
			assert((isDirect() || isExtend()) && "Not a direct or extend kind");
			return directOffset;
		}
		
		void setDirectOffset(unsigned offset) {
			assert((isDirect() || isExtend()) && "Not a direct or extend kind");
			directOffset = offset;
		}
		
		Type getPaddingType() const { return paddingType; }
		
		void setPaddingType(Type type) { paddingType = type; }
		
		bool getPaddingInReg() const {
			return paddingInReg;
		}
		
		void setPaddingInReg(bool PIR) {
			paddingInReg = PIR;
		}
		
		Type getCoerceToType() const {
			assert(canHaveCoerceToType() && "Invalid kind!");
			return typeData;
		}

		void setCoerceToType(Type type) {
			assert(canHaveCoerceToType() && "Invalid kind!");
			typeData = type;
		}
		
		Type getExpandType() const {
			assert(isExpand() && "Invalid kind!");
			return typeData;
		}

		void setExpandType(const Type type) {
			assert(isExpand() && "Invalid kind!");
			typeData = type;
		}
		
		bool getInReg() const {
			assert((isDirect() || isExtend() || isIndirect()) && "Invalid kind!");
			return inReg;
		}
		
		void setInReg(bool newInReg) {
			assert((isDirect() || isExtend() || isIndirect()) && "Invalid kind!");
			inReg = newInReg;
		}
		
		// Indirect accessors
		unsigned getIndirectAlign() const {
			assert(isIndirect() && "Invalid kind!");
			return indirectAlign;
		}
		
		void setIndirectAlign(unsigned IA) {
			assert(isIndirect() && "Invalid kind!");
			indirectAlign = IA;
		}

		bool getIndirectByVal() const {
			assert(isIndirect() && "Invalid kind!");
			return indirectByVal;
		}
		
		void setIndirectByVal(unsigned IBV) {
			assert(isIndirect() && "Invalid kind!");
			indirectByVal = IBV;
		}

		bool getIndirectRealign() const {
			assert(isIndirect() && "Invalid kind!");
			return indirectRealign;
		}
		
		void setIndirectRealign(bool IR) {
			assert(isIndirect() && "Invalid kind!");
			indirectRealign = IR;
		}

		bool isSRetAfterThis() const {
			assert(isIndirect() && "Invalid kind!");
			return sRetAfterThis;
		}
		
		void setSRetAfterThis(bool afterThis) {
			assert(isIndirect() && "Invalid kind!");
			sRetAfterThis = afterThis;
		}

		unsigned getInAllocaFieldIndex() const {
			assert(isInAlloca() && "Invalid kind!");
			return allocaFieldIndex;
		}
		
		void setInAllocaFieldIndex(unsigned fieldIndex) {
			assert(isInAlloca() && "Invalid kind!");
			allocaFieldIndex = fieldIndex;
		}
		
		/// \brief Return true if this field of an inalloca struct should be returned
		/// to implement a struct return calling convention.
		bool getInAllocaSRet() const {
			assert(isInAlloca() && "Invalid kind!");
			return inAllocaSRet;
		}
		
		void setinAllocaSRet(bool sRet) {
			assert(isInAlloca() && "Invalid kind!");
			inAllocaSRet = sRet;
		}
		
		bool getCanBeFlattened() const {
			assert(isDirect() && "Invalid kind!");
			return canBeFlattened;
		}

		void setCanBeFlattened(bool flatten) {
			assert(isDirect() && "Invalid kind!");
			canBeFlattened = flatten;
		}
		
		std::string toString() const {
			std::ostringstream OS;
			OS << "(ArgInfo Kind=";
			switch (kind) {
			case Direct:
				OS << "Direct Type=";
				OS << getCoerceToType().toString();
				break;
			case ExtendInteger:
				OS << "Extend";
				break;
			case Ignore:
				OS << "Ignore";
				break;
			case InAlloca:
				OS << "InAlloca Offset=" << getInAllocaFieldIndex();
				break;
			case Indirect:
				OS << "Indirect Align=" << getIndirectAlign()
					 << " ByVal=" << getIndirectByVal()
					 << " Realign=" << getIndirectRealign();
				break;
			case Expand:
				OS << "Expand";
				break;
			}
			OS << ")\n";
			return OS.str();
		}
	};
	
}

#endif
