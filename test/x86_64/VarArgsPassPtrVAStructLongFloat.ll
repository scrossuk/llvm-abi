; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void (ptr, ...({ long, float }))

declare void @callee(i8*, ...)

define void @caller(i8*, i64 %coerce0, float %coerce1) {
  %coerce.arg.source = alloca { i64, float }, align 8
  %coerce.mem = alloca { i64, float }, align 8
  %2 = getelementptr { i64, float }* %coerce.mem, i32 0, i32 0
  store i64 %coerce0, i64* %2
  %3 = getelementptr { i64, float }* %coerce.mem, i32 0, i32 1
  store float %coerce1, float* %3
  %4 = load { i64, float }* %coerce.mem
  store { i64, float } %4, { i64, float }* %coerce.arg.source
  %5 = getelementptr { i64, float }* %coerce.arg.source, i32 0, i32 0
  %6 = load i64* %5, align 1
  %7 = getelementptr { i64, float }* %coerce.arg.source, i32 0, i32 1
  %8 = load float* %7, align 1
  call void (i8*, ...)* @callee(i8* %0, i64 %6, float %8)
  ret void
}
