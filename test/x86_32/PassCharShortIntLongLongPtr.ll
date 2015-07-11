; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void (char, short, int, longlong, ptr)

declare void @callee(i8 signext, i16 signext, i32, i64, i8*)

define void @caller(i8 signext, i16 signext, i32, i64, i8*) {
  call void @callee(i8 signext %0, i16 signext %1, i32 %2, i64 %3, i8* %4)
  ret void
}
