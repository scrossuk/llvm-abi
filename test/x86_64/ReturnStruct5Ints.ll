; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: {int, int, int, int, int} ()

declare void @callee({ i32, i32, i32, i32, i32 }*)

define void @caller({ i32, i32, i32, i32, i32 }* %agg.result) {
  %1 = alloca { i32, i32, i32, i32, i32 }
  call void @callee({ i32, i32, i32, i32, i32 }* %1)
  %2 = load { i32, i32, i32, i32, i32 }* %1
  store { i32, i32, i32, i32, i32 } %2, { i32, i32, i32, i32, i32 }* %agg.result
  ret void
}
