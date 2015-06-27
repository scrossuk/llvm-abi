; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: {long, int} ()

declare { i64, i32 } @callee()

define { i64, i32 } @caller() {
  %coerce1 = alloca { i64, i32 }
  %coerce = alloca { i64, i32 }, align 8
  %1 = call { i64, i32 } @callee()
  store { i64, i32 } %1, { i64, i32 }* %coerce
  %2 = load { i64, i32 }* %coerce
  store { i64, i32 } %2, { i64, i32 }* %coerce1
  %3 = load { i64, i32 }* %coerce1
  ret { i64, i32 } %3
}
