; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void (float)

declare void @callee(float)

define void @caller(float) {
  call void @callee(float %0)
  ret void
}
