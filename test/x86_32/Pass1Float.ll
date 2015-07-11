; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void (float)

declare void @callee(float)

define void @caller(float) {
  call void @callee(float %0)
  ret void
}
