; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void (int)

declare void @callee(i32)

define void @caller(i32) {
entry:
  call void @callee(i32 %0)
  ret void
}
