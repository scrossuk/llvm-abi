; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: {int} ()

declare i32 @callee()

define i32 @caller() {
  %coerce1 = alloca { i32 }
  %coerce = alloca { i32 }, align 4
  %1 = call i32 @callee()
  %coerce.dive = getelementptr { i32 }* %coerce, i32 0, i32 0
  store i32 %1, i32* %coerce.dive
  %2 = load { i32 }* %coerce
  store { i32 } %2, { i32 }* %coerce1
  %coerce.dive2 = getelementptr { i32 }* %coerce1, i32 0, i32 0
  %3 = load i32* %coerce.dive2
  ret i32 %3
}
