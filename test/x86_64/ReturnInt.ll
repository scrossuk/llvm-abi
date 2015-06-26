; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: int ()

declare i32 @callee()

define i32 @caller() {
  %1 = call i32 @callee()
  ret i32 %1
}
