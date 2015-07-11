; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void (int, int, int, int, int, int, ptr)

declare void @callee(i32, i32, i32, i32, i32, i32, i8*)

define void @caller(i32, i32, i32, i32, i32, i32, i8*) {
  call void @callee(i32 %0, i32 %1, i32 %2, i32 %3, i32 %4, i32 %5, i8* %6)
  ret void
}
