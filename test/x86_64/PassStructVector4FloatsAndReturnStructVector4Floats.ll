; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: {<4 x float>} ({<4 x float>})

declare <4 x float> @callee(<4 x float>)

define <4 x float> @caller(<4 x float> %coerce) {
  %coerce4 = alloca { <4 x float> }, align 16
  %coerce2 = alloca { <4 x float> }, align 16
  %coerce.arg.source = alloca { <4 x float> }, align 16
  %coerce.mem = alloca { <4 x float> }, align 16
  %coerce.dive = getelementptr { <4 x float> }* %coerce.mem, i32 0, i32 0
  store <4 x float> %coerce, <4 x float>* %coerce.dive, align 1
  %1 = load { <4 x float> }* %coerce.mem
  store { <4 x float> } %1, { <4 x float> }* %coerce.arg.source
  %coerce.dive1 = getelementptr { <4 x float> }* %coerce.arg.source, i32 0, i32 0
  %2 = load <4 x float>* %coerce.dive1, align 1
  %3 = call <4 x float> @callee(<4 x float> %2)
  %coerce.dive3 = getelementptr { <4 x float> }* %coerce2, i32 0, i32 0
  store <4 x float> %3, <4 x float>* %coerce.dive3, align 1
  %4 = load { <4 x float> }* %coerce2
  store { <4 x float> } %4, { <4 x float> }* %coerce4
  %coerce.dive5 = getelementptr { <4 x float> }* %coerce4, i32 0, i32 0
  %5 = load <4 x float>* %coerce.dive5, align 1
  ret <4 x float> %5
}
