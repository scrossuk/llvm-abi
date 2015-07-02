; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: <8 x float> (<8 x float>)

declare <8 x float> @callee(<8 x float>* byval align 32)

define <8 x float> @caller(<8 x float>* byval align 32) {
  %indirect.arg.mem = alloca <8 x float>, align 32
  %2 = load <8 x float>* %0, align 32
  store <8 x float> %2, <8 x float>* %indirect.arg.mem, align 32
  %3 = call <8 x float> @callee(<8 x float>* byval align 32 %indirect.arg.mem)
  ret <8 x float> %3
}
