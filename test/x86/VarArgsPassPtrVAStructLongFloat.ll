; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void (ptr, ...({ long, float }))

declare void @callee(i8*, ...)

define void @caller(i8*, i32, float) {
  %expand.source.arg = alloca { i32, float }, align 4
  %expand.dest.arg = alloca { i32, float }, align 4
  %4 = getelementptr { i32, float }* %expand.dest.arg, i32 0, i32 0
  store i32 %1, i32* %4, align 4
  %5 = getelementptr { i32, float }* %expand.dest.arg, i32 0, i32 1
  store float %2, float* %5, align 4
  %6 = load { i32, float }* %expand.dest.arg, align 4
  store { i32, float } %6, { i32, float }* %expand.source.arg, align 4
  %7 = getelementptr { i32, float }* %expand.source.arg, i32 0, i32 0
  %8 = load i32* %7, align 4
  %9 = getelementptr { i32, float }* %expand.source.arg, i32 0, i32 1
  %10 = load float* %9, align 4
  call void (i8*, ...)* @callee(i8* %0, i32 %8, float %10)
  ret void
}
