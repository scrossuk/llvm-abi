; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: schar ()

declare signext i8 @callee()

define signext i8 @caller() {
  %1 = call signext i8 @callee()
  ret i8 %1
}
