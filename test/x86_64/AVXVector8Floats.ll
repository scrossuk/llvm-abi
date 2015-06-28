; ABI: x86_64-none-linux-gnu
; Sandy Bridge has the AVX support this test needs.
; CPU: sandybridge
; FUNCTION-TYPE: <8 x float> (<8 x float>)

declare <8 x float> @callee(<8 x float>)

define <8 x float> @caller(<8 x float>) {
  %2 = call <8 x float> @callee(<8 x float> %0)
  ret <8 x float> %2
}
