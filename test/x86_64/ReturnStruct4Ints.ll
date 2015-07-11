; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: {int, int, int, int} ()

declare { i64, i64 } @callee()

define { i64, i64 } @caller() {
  %coerce1 = alloca { i32, i32, i32, i32 }, align 4
  %coerce = alloca { i32, i32, i32, i32 }, align 4
  %1 = call { i64, i64 } @callee()
  %2 = bitcast { i32, i32, i32, i32 }* %coerce to { i64, i64 }*
  %3 = getelementptr { i64, i64 }* %2, i32 0, i32 0
  %4 = extractvalue { i64, i64 } %1, 0
  store i64 %4, i64* %3, align 1
  %5 = getelementptr { i64, i64 }* %2, i32 0, i32 1
  %6 = extractvalue { i64, i64 } %1, 1
  store i64 %6, i64* %5, align 1
  %7 = load { i32, i32, i32, i32 }* %coerce, align 4
  store { i32, i32, i32, i32 } %7, { i32, i32, i32, i32 }* %coerce1, align 4
  %8 = bitcast { i32, i32, i32, i32 }* %coerce1 to { i64, i64 }*
  %9 = load { i64, i64 }* %8, align 1
  ret { i64, i64 } %9
}
