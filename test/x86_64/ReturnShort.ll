; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: short ()

declare i16 @callee()

define i16 @caller() {
entry:
  %0 = call i16 @callee()
  ret i16 %0
}
