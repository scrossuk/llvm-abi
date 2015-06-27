; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: {float, float} ()

declare <2 x float> @callee()

define <2 x float> @caller() {
  %coerce1 = alloca { float, float }, align 4
  %coerce = alloca { float, float }, align 4
  %1 = call <2 x float> @callee()
  %2 = bitcast { float, float }* %coerce to <2 x float>*
  store <2 x float> %1, <2 x float>* %2, align 1
  %3 = load { float, float }* %coerce
  store { float, float } %3, { float, float }* %coerce1
  %4 = bitcast { float, float }* %coerce1 to <2 x float>*
  %5 = load <2 x float>* %4, align 1
  ret <2 x float> %5
}
