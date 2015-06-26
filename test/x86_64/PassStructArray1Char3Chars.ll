; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void ({[8 x char], char, char, char})

declare void @callee(i64, i24)

define void @caller(i64 %coerce0, i24 %coerce1) {
  %coerce.arg.source.coerce = alloca { i64, i24 }
  %coerce.arg.source = alloca { [8 x i8], i8, i8, i8 }, align 1
  %coerce = alloca { i64, i24 }, align 8
  %coerce.mem = alloca { [8 x i8], i8, i8, i8 }, align 8
  %1 = getelementptr { i64, i24 }* %coerce, i32 0, i32 0
  store i64 %coerce0, i64* %1
  %2 = getelementptr { i64, i24 }* %coerce, i32 0, i32 1
  store i24 %coerce1, i24* %2
  %3 = bitcast { [8 x i8], i8, i8, i8 }* %coerce.mem to i8*
  %4 = bitcast { i64, i24 }* %coerce to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %3, i8* %4, i64 11, i32 8, i1 false)
  %5 = load { [8 x i8], i8, i8, i8 }* %coerce.mem
  store { [8 x i8], i8, i8, i8 } %5, { [8 x i8], i8, i8, i8 }* %coerce.arg.source
  %6 = bitcast { i64, i24 }* %coerce.arg.source.coerce to i8*
  %7 = bitcast { [8 x i8], i8, i8, i8 }* %coerce.arg.source to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %6, i8* %7, i64 11, i32 0, i1 false)
  %8 = getelementptr { i64, i24 }* %coerce.arg.source.coerce, i32 0, i32 0
  %9 = load i64* %8, align 1
  %10 = getelementptr { i64, i24 }* %coerce.arg.source.coerce, i32 0, i32 1
  %11 = load i24* %10, align 1
  call void @callee(i64 %9, i24 %11)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #0

attributes #0 = { nounwind }
