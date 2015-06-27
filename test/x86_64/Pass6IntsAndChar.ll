; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void (int, int, int, int, int, int, char)

declare void @callee(i32, i32, i32, i32, i32, i32, i8 signext)

define void @caller(i32, i32, i32, i32, i32, i32, i8 signext) {
  call void @callee(i32 %0, i32 %1, i32 %2, i32 %3, i32 %4, i32 %5, i8 signext %6)
  ret void
}
