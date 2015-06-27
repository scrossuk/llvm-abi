; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: {int, int} ()

declare i64 @callee()

define i64 @caller() {
  %coerce1 = alloca { i32, i32 }, align 4
  %coerce = alloca { i32, i32 }, align 4
  %1 = call i64 @callee()
  %2 = bitcast { i32, i32 }* %coerce to i64*
  store i64 %1, i64* %2, align 1
  %3 = load { i32, i32 }* %coerce
  store { i32, i32 } %3, { i32, i32 }* %coerce1
  %4 = bitcast { i32, i32 }* %coerce1 to i64*
  %5 = load i64* %4, align 1
  ret i64 %5
}
