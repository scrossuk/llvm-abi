; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: uchar ()

declare zeroext i8 @callee()

define zeroext i8 @caller() {
  %1 = call zeroext i8 @callee()
  ret i8 %1
}
