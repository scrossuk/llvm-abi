; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void ({ float })

declare void @callee(float)

define void @caller(float %coerce) {
  %coerce.arg.source = alloca { float }, align 4
  %coerce.mem = alloca { float }, align 4
  %coerce.dive = getelementptr { float }* %coerce.mem, i32 0, i32 0
  store float %coerce, float* %coerce.dive, align 1
  %1 = load { float }* %coerce.mem
  store { float } %1, { float }* %coerce.arg.source
  %coerce.dive1 = getelementptr { float }* %coerce.arg.source, i32 0, i32 0
  %2 = load float* %coerce.dive1, align 1
  call void @callee(float %2)
  ret void
}
