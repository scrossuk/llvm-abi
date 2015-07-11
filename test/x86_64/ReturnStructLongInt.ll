; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: {long, int} ()

declare { i64, i32 } @callee()

define { i64, i32 } @caller() {
  %agg.tmp = alloca { i64, i32 }, align 8
  %1 = call { i64, i32 } @callee()
  %2 = getelementptr { i64, i32 }* %agg.tmp, i32 0, i32 0
  %3 = extractvalue { i64, i32 } %1, 0
  store i64 %3, i64* %2
  %4 = getelementptr { i64, i32 }* %agg.tmp, i32 0, i32 1
  %5 = extractvalue { i64, i32 } %1, 1
  store i32 %5, i32* %4
  %6 = load { i64, i32 }* %agg.tmp, align 8
  ret { i64, i32 } %6
}
