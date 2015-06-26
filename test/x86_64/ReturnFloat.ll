; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: float ()

declare float @callee()

define float @caller() {
entry:
  %0 = call float @callee()
  ret float %0
}
