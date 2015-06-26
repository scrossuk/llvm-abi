; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: short ()

declare signext i16 @callee()

define signext i16 @caller() {
  %1 = call signext i16 @callee()
  ret i16 %1
}
