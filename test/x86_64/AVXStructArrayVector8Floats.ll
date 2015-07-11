; ABI: x86_64-none-linux-gnu
; Sandy Bridge has the AVX support this test needs.
; CPU: sandybridge
; FUNCTION-TYPE: {[1 x <8 x float>]} ({[1 x <8 x float>]})

declare <8 x float> @callee(<8 x float>)

define <8 x float> @caller(<8 x float> %coerce) {
  %coerce4 = alloca { [1 x <8 x float>] }, align 32
  %coerce2 = alloca { [1 x <8 x float>] }, align 32
  %coerce.arg.source = alloca { [1 x <8 x float>] }, align 32
  %coerce.mem = alloca { [1 x <8 x float>] }, align 32
  %coerce.dive = getelementptr { [1 x <8 x float>] }* %coerce.mem, i32 0, i32 0
  %1 = bitcast [1 x <8 x float>]* %coerce.dive to <8 x float>*
  store <8 x float> %coerce, <8 x float>* %1, align 1
  %2 = load { [1 x <8 x float>] }* %coerce.mem
  store { [1 x <8 x float>] } %2, { [1 x <8 x float>] }* %coerce.arg.source
  %coerce.dive1 = getelementptr { [1 x <8 x float>] }* %coerce.arg.source, i32 0, i32 0
  %3 = bitcast [1 x <8 x float>]* %coerce.dive1 to <8 x float>*
  %4 = load <8 x float>* %3, align 1
  %5 = call <8 x float> @callee(<8 x float> %4)
  %coerce.dive3 = getelementptr { [1 x <8 x float>] }* %coerce2, i32 0, i32 0
  %6 = bitcast [1 x <8 x float>]* %coerce.dive3 to <8 x float>*
  store <8 x float> %5, <8 x float>* %6, align 1
  %7 = load { [1 x <8 x float>] }* %coerce2, align 32
  store { [1 x <8 x float>] } %7, { [1 x <8 x float>] }* %coerce4, align 32
  %coerce.dive5 = getelementptr { [1 x <8 x float>] }* %coerce4, i32 0, i32 0
  %8 = bitcast [1 x <8 x float>]* %coerce.dive5 to <8 x float>*
  %9 = load <8 x float>* %8, align 1
  ret <8 x float> %9
}
