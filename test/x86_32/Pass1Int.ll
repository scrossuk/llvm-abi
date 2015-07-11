; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void (int)

declare void @callee(i32)

define void @caller(i32) {
  call void @callee(i32 %0)
  ret void
}
