; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void ({ long, ptr })

declare void @callee(i64, i8*)

define void @caller(i64 %coerce0, i8* %coerce1) {
  %coerce.arg.source = alloca { i64, i8* }, align 8
  %coerce.mem = alloca { i64, i8* }, align 8
  %1 = getelementptr { i64, i8* }* %coerce.mem, i32 0, i32 0
  store i64 %coerce0, i64* %1
  %2 = getelementptr { i64, i8* }* %coerce.mem, i32 0, i32 1
  store i8* %coerce1, i8** %2
  %3 = load { i64, i8* }* %coerce.mem
  store { i64, i8* } %3, { i64, i8* }* %coerce.arg.source
  %4 = getelementptr { i64, i8* }* %coerce.arg.source, i32 0, i32 0
  %5 = load i64* %4, align 1
  %6 = getelementptr { i64, i8* }* %coerce.arg.source, i32 0, i32 1
  %7 = load i8** %6, align 1
  call void @callee(i64 %5, i8* %7)
  ret void
}
