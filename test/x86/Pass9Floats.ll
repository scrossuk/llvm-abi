; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void (float, float, float, float, float, float, float, float, float)

declare void @callee(float, float, float, float, float, float, float, float, float)

define void @caller(float, float, float, float, float, float, float, float, float) {
  call void @callee(float %0, float %1, float %2, float %3, float %4, float %5, float %6, float %7, float %8)
  ret void
}
