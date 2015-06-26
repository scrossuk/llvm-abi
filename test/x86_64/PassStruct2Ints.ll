; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void ({ int, int })

declare void @callee(i64)

define void @caller(i64 %coerce) {
  %coerce.arg.source = alloca { i32, i32 }, align 4
  %coerce.mem = alloca { i32, i32 }, align 8
  %1 = bitcast { i32, i32 }* %coerce.mem to i64*
  store i64 %coerce, i64* %1, align 1
  %2 = load { i32, i32 }* %coerce.mem
  store { i32, i32 } %2, { i32, i32 }* %coerce.arg.source
  %3 = bitcast { i32, i32 }* %coerce.arg.source to i64*
  %4 = load i64* %3, align 1
  call void @callee(i64 %4)
  ret void
}
