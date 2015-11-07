; ABI: i386-apple-darwin9
; FUNCTION-TYPE: {[0 x char], float} ()

declare float @callee()

define float @caller() {
  %coerce1 = alloca { [0 x i8], float }, align 4
  %coerce = alloca { [0 x i8], float }, align 4
  %1 = call float @callee()
  %2 = bitcast { [0 x i8], float }* %coerce to float*
  store float %1, float* %2, align 1
  %3 = load { [0 x i8], float }* %coerce, align 4
  store { [0 x i8], float } %3, { [0 x i8], float }* %coerce1, align 4
  %4 = bitcast { [0 x i8], float }* %coerce1 to float*
  %5 = load float* %4, align 1
  ret float %5
}
