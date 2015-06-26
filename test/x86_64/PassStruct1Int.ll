; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void ({ int })

declare void @callee(i32)

define void @caller(i32 %coerce) {
  %coerce.arg.source = alloca { i32 }, align 4
  %coerce.mem = alloca { i32 }, align 4
  %coerce.dive = getelementptr { i32 }* %coerce.mem, i32 0, i32 0
  store i32 %coerce, i32* %coerce.dive
  %1 = load { i32 }* %coerce.mem
  store { i32 } %1, { i32 }* %coerce.arg.source
  %coerce.dive1 = getelementptr { i32 }* %coerce.arg.source, i32 0, i32 0
  %2 = load i32* %coerce.dive1
  call void @callee(i32 %2)
  ret void
}
