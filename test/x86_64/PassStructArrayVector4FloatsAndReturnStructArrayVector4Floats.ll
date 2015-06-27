; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: {[1 x <4 x float>]} ({[1 x <4 x float>]})

declare <4 x float> @callee(<4 x float>)

define <4 x float> @caller(<4 x float> %coerce) {
  %coerce4 = alloca { [1 x <4 x float>] }, align 16
  %coerce2 = alloca { [1 x <4 x float>] }, align 16
  %coerce.arg.source = alloca { [1 x <4 x float>] }, align 16
  %coerce.mem = alloca { [1 x <4 x float>] }, align 16
  %coerce.dive = getelementptr { [1 x <4 x float>] }* %coerce.mem, i32 0, i32 0
  %1 = bitcast [1 x <4 x float>]* %coerce.dive to <4 x float>*
  store <4 x float> %coerce, <4 x float>* %1, align 1
  %2 = load { [1 x <4 x float>] }* %coerce.mem
  store { [1 x <4 x float>] } %2, { [1 x <4 x float>] }* %coerce.arg.source
  %coerce.dive1 = getelementptr { [1 x <4 x float>] }* %coerce.arg.source, i32 0, i32 0
  %3 = bitcast [1 x <4 x float>]* %coerce.dive1 to <4 x float>*
  %4 = load <4 x float>* %3, align 1
  %5 = call <4 x float> @callee(<4 x float> %4)
  %coerce.dive3 = getelementptr { [1 x <4 x float>] }* %coerce2, i32 0, i32 0
  %6 = bitcast [1 x <4 x float>]* %coerce.dive3 to <4 x float>*
  store <4 x float> %5, <4 x float>* %6, align 1
  %7 = load { [1 x <4 x float>] }* %coerce2
  store { [1 x <4 x float>] } %7, { [1 x <4 x float>] }* %coerce4
  %coerce.dive5 = getelementptr { [1 x <4 x float>] }* %coerce4, i32 0, i32 0
  %8 = bitcast [1 x <4 x float>]* %coerce.dive5 to <4 x float>*
  %9 = load <4 x float>* %8, align 1
  ret <4 x float> %9
}
