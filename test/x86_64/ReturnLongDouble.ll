; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: longdouble ()

declare x86_fp80 @callee()

define x86_fp80 @caller() {
  %1 = call x86_fp80 @callee()
  ret x86_fp80 %1
}
