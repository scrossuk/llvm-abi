; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void (float, float, float, float, float, float, float, float, longdouble)

declare void @callee(float, float, float, float, float, float, float, float, x86_fp80)

define void @caller(float, float, float, float, float, float, float, float, x86_fp80) {
  call void @callee(float %0, float %1, float %2, float %3, float %4, float %5, float %6, float %7, x86_fp80 %8)
  ret void
}
