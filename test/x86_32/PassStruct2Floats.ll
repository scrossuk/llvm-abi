; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void ({ float, float })

declare void @callee(float, float)

define void @caller(float, float) {
  %expand.source.arg = alloca { float, float }, align 4
  %expand.dest.arg = alloca { float, float }, align 4
  %3 = getelementptr { float, float }* %expand.dest.arg, i32 0, i32 0
  store float %0, float* %3, align 4
  %4 = getelementptr { float, float }* %expand.dest.arg, i32 0, i32 1
  store float %1, float* %4, align 4
  %5 = load { float, float }* %expand.dest.arg, align 4
  store { float, float } %5, { float, float }* %expand.source.arg, align 4
  %6 = getelementptr { float, float }* %expand.source.arg, i32 0, i32 0
  %7 = load float* %6, align 4
  %8 = getelementptr { float, float }* %expand.source.arg, i32 0, i32 1
  %9 = load float* %8, align 4
  call void @callee(float %7, float %9)
  ret void
}
