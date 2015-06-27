; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: <4 x float> (<4 x float>)

declare <4 x float> @callee(<4 x float>)

define <4 x float> @caller(<4 x float>) {
  %2 = call <4 x float> @callee(<4 x float> %0)
  ret <4 x float> %2
}
