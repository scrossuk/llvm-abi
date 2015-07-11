; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: ptr ({ long, ptr })

declare i8* @callee(i32, i8*)

define i8* @caller(i32, i8*) {
  %expand.source.arg = alloca { i32, i8* }, align 4
  %expand.dest.arg = alloca { i32, i8* }, align 4
  %3 = getelementptr { i32, i8* }* %expand.dest.arg, i32 0, i32 0
  store i32 %0, i32* %3, align 4
  %4 = getelementptr { i32, i8* }* %expand.dest.arg, i32 0, i32 1
  store i8* %1, i8** %4, align 4
  %5 = load { i32, i8* }* %expand.dest.arg, align 4
  store { i32, i8* } %5, { i32, i8* }* %expand.source.arg, align 4
  %6 = getelementptr { i32, i8* }* %expand.source.arg, i32 0, i32 0
  %7 = load i32* %6, align 4
  %8 = getelementptr { i32, i8* }* %expand.source.arg, i32 0, i32 1
  %9 = load i8** %8, align 4
  %10 = call i8* @callee(i32 %7, i8* %9)
  ret i8* %10
}
