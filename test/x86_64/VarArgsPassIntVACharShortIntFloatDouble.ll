; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void (int, ...(char, short, int, float, double))

declare void @callee(i32, ...)

define void @caller(i32, i8 signext, i16 signext, i32, float, double) {
  %7 = sext i8 %1 to i32
  %8 = sext i16 %2 to i32
  %9 = fpext float %4 to double
  call void (i32, ...)* @callee(i32 %0, i32 %7, i32 %8, i32 %3, double %9, double %5)
  ret void
}
