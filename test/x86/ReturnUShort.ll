; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: ushort ()

declare zeroext i16 @callee()

define zeroext i16 @caller() {
  %1 = call zeroext i16 @callee()
  ret i16 %1
}
