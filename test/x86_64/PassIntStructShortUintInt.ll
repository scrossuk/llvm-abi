; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void (int, {short, uint, int})

declare void @callee(i32, i64, i32)

define void @caller(i32, i64 %coerce0, i32 %coerce1) {
  %coerce.arg.source.coerce = alloca { i64, i32 }
  %coerce.arg.source = alloca { i16, i32, i32 }, align 4
  %coerce = alloca { i64, i32 }, align 8
  %coerce.mem = alloca { i16, i32, i32 }, align 8
  %2 = getelementptr { i64, i32 }* %coerce, i32 0, i32 0
  store i64 %coerce0, i64* %2
  %3 = getelementptr { i64, i32 }* %coerce, i32 0, i32 1
  store i32 %coerce1, i32* %3
  %4 = bitcast { i16, i32, i32 }* %coerce.mem to i8*
  %5 = bitcast { i64, i32 }* %coerce to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %4, i8* %5, i64 12, i32 8, i1 false)
  %6 = load { i16, i32, i32 }* %coerce.mem
  store { i16, i32, i32 } %6, { i16, i32, i32 }* %coerce.arg.source
  %7 = bitcast { i64, i32 }* %coerce.arg.source.coerce to i8*
  %8 = bitcast { i16, i32, i32 }* %coerce.arg.source to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %7, i8* %8, i64 12, i32 0, i1 false)
  %9 = getelementptr { i64, i32 }* %coerce.arg.source.coerce, i32 0, i32 0
  %10 = load i64* %9, align 1
  %11 = getelementptr { i64, i32 }* %coerce.arg.source.coerce, i32 0, i32 1
  %12 = load i32* %11, align 1
  call void @callee(i32 %0, i64 %10, i32 %12)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #0

attributes #0 = { argmemonly nounwind }
