; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: char ()

declare signext i8 @callee()

define signext i8 @caller() {
entry:
  %0 = call signext i8 @callee()
  ret i8 %0
}
