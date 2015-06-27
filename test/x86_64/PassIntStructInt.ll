; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void (int, { int })
; 
; Check for valid coercion.  The struct should be passed/returned as i32, not as
; i64 for better code quality.

declare void @callee(i32, i32)

define void @caller(i32, i32 %coerce) {
  %coerce.arg.source = alloca { i32 }, align 4
  %coerce.mem = alloca { i32 }, align 4
  %coerce.dive = getelementptr { i32 }* %coerce.mem, i32 0, i32 0
  store i32 %coerce, i32* %coerce.dive
  %2 = load { i32 }* %coerce.mem
  store { i32 } %2, { i32 }* %coerce.arg.source
  %coerce.dive1 = getelementptr { i32 }* %coerce.arg.source, i32 0, i32 0
  %3 = load i32* %coerce.dive1
  call void @callee(i32 %0, i32 %3)
  ret void
}
