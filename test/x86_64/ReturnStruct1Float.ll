; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: {float} ()

declare float @callee()

define float @caller() {
  %coerce1 = alloca { float }
  %coerce = alloca { float }, align 4
  %1 = call float @callee()
  %coerce.dive = getelementptr { float }* %coerce, i32 0, i32 0
  store float %1, float* %coerce.dive, align 1
  %2 = load { float }* %coerce
  store { float } %2, { float }* %coerce1
  %coerce.dive2 = getelementptr { float }* %coerce1, i32 0, i32 0
  %3 = load float* %coerce.dive2, align 1
  ret float %3
}
