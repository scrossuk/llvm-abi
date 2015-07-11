; ABI: x86_64-none-linux-gnu
; Sandy Bridge has the AVX support this test needs.
; CPU: sandybridge
; FUNCTION-TYPE: {<8 x float>} ({<8 x float>})

declare <8 x float> @callee(<8 x float>)

define <8 x float> @caller(<8 x float> %coerce) {
  %coerce4 = alloca { <8 x float> }, align 32
  %coerce2 = alloca { <8 x float> }, align 32
  %coerce.arg.source = alloca { <8 x float> }, align 32
  %coerce.mem = alloca { <8 x float> }, align 32
  %coerce.dive = getelementptr { <8 x float> }* %coerce.mem, i32 0, i32 0
  store <8 x float> %coerce, <8 x float>* %coerce.dive, align 1
  %1 = load { <8 x float> }* %coerce.mem
  store { <8 x float> } %1, { <8 x float> }* %coerce.arg.source
  %coerce.dive1 = getelementptr { <8 x float> }* %coerce.arg.source, i32 0, i32 0
  %2 = load <8 x float>* %coerce.dive1, align 1
  %3 = call <8 x float> @callee(<8 x float> %2)
  %coerce.dive3 = getelementptr { <8 x float> }* %coerce2, i32 0, i32 0
  store <8 x float> %3, <8 x float>* %coerce.dive3, align 1
  %4 = load { <8 x float> }* %coerce2, align 32
  store { <8 x float> } %4, { <8 x float> }* %coerce4, align 32
  %coerce.dive5 = getelementptr { <8 x float> }* %coerce4, i32 0, i32 0
  %5 = load <8 x float>* %coerce.dive5, align 1
  ret <8 x float> %5
}
