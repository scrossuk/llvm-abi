; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void ({ int, int, int })

declare void @callee(i64, i32)

define void @caller(i64 %coerce0, i32 %coerce1) {
  %coerce.arg.source.coerce = alloca { i64, i32 }
  %coerce.arg.source = alloca { i32, i32, i32 }, align 4
  %coerce = alloca { i64, i32 }, align 8
  %coerce.mem = alloca { i32, i32, i32 }, align 8
  %1 = getelementptr { i64, i32 }* %coerce, i32 0, i32 0
  store i64 %coerce0, i64* %1
  %2 = getelementptr { i64, i32 }* %coerce, i32 0, i32 1
  store i32 %coerce1, i32* %2
  %3 = bitcast { i32, i32, i32 }* %coerce.mem to i8*
  %4 = bitcast { i64, i32 }* %coerce to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %3, i8* %4, i64 12, i32 8, i1 false)
  %5 = load { i32, i32, i32 }* %coerce.mem
  store { i32, i32, i32 } %5, { i32, i32, i32 }* %coerce.arg.source
  %6 = bitcast { i64, i32 }* %coerce.arg.source.coerce to i8*
  %7 = bitcast { i32, i32, i32 }* %coerce.arg.source to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %6, i8* %7, i64 12, i32 0, i1 false)
  %8 = getelementptr { i64, i32 }* %coerce.arg.source.coerce, i32 0, i32 0
  %9 = load i64* %8, align 1
  %10 = getelementptr { i64, i32 }* %coerce.arg.source.coerce, i32 0, i32 1
  %11 = load i32* %10, align 1
  call void @callee(i64 %9, i32 %11)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #0

attributes #0 = { argmemonly nounwind }
