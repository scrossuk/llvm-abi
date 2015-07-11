; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: float ()

declare float @callee()

define float @caller() {
  %1 = call float @callee()
  ret float %1
}
