; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void (ptr, ...(ptr, int, double))

declare void @callee(i8*, ...)

define void @caller(i8*, i8*, i32, double) {
  call void (i8*, ...)* @callee(i8* %0, i8* %1, i32 %2, double %3)
  ret void
}
