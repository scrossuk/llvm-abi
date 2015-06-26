; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: char ()

declare i8 @callee()

define i8 @caller() {
entry:
  %0 = call i8 @callee()
  ret i8 %0
}
