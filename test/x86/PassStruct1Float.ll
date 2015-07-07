; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void ({ float })

declare void @callee(float)

define void @caller(float) {
  %expand.source.arg = alloca { float }, align 4
  %expand.dest.arg = alloca { float }, align 4
  %2 = getelementptr { float }* %expand.dest.arg, i32 0, i32 0
  store float %0, float* %2, align 4
  %3 = load { float }* %expand.dest.arg, align 4
  store { float } %3, { float }* %expand.source.arg, align 4
  %4 = getelementptr { float }* %expand.source.arg, i32 0, i32 0
  %5 = load float* %4, align 4
  call void @callee(float %5)
  ret void
}
