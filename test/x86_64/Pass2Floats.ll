; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void (float, float)

declare void @callee(float, float)

define void @caller(float, float) {
  call void @callee(float %0, float %1)
  ret void
}
