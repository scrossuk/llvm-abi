; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void (int, ...(int))

declare void @callee(i32, ...)

define void @caller(i32, i32) {
  call void (i32, ...)* @callee(i32 %0, i32 %1)
  ret void
}
