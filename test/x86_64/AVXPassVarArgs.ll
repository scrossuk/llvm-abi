; ABI: x86_64-none-linux-gnu
; Sandy Bridge has the AVX support this test needs.
; CPU: sandybridge
; FUNCTION-TYPE: void (int, ...(<8 x float>))

declare void @callee(i32, ...)

define void @caller(i32, <8 x float>) {
  call void (i32, ...)* @callee(i32 %0, <8 x float> %1)
  ret void
}
