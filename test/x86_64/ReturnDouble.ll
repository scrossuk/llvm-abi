; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: double ()

declare double @callee()

define double @caller() {
entry:
  %0 = call double @callee()
  ret double %0
}
