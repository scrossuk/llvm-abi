; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void ({ int, int, int, int })

declare void @callee(i64, i64)

define void @caller(i64 %coerce0, i64 %coerce1) {
  %coerce.arg.source = alloca { i32, i32, i32, i32 }, align 4
  %coerce.mem = alloca { i32, i32, i32, i32 }, align 8
  %1 = bitcast { i32, i32, i32, i32 }* %coerce.mem to { i64, i64 }*
  %2 = getelementptr { i64, i64 }* %1, i32 0, i32 0
  store i64 %coerce0, i64* %2
  %3 = getelementptr { i64, i64 }* %1, i32 0, i32 1
  store i64 %coerce1, i64* %3
  %4 = load { i32, i32, i32, i32 }* %coerce.mem
  store { i32, i32, i32, i32 } %4, { i32, i32, i32, i32 }* %coerce.arg.source
  %5 = bitcast { i32, i32, i32, i32 }* %coerce.arg.source to { i64, i64 }*
  %6 = getelementptr { i64, i64 }* %5, i32 0, i32 0
  %7 = load i64* %6, align 1
  %8 = getelementptr { i64, i64 }* %5, i32 0, i32 1
  %9 = load i64* %8, align 1
  call void @callee(i64 %7, i64 %9)
  ret void
}
