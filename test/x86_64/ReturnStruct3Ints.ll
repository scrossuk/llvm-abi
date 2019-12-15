; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: {int, int, int} ()

declare { i64, i32 } @callee()

define { i64, i32 } @caller() {
  %coerce.mem.load = alloca { i64, i32 }
  %coerce1 = alloca { i32, i32, i32 }, align 4
  %coerce.mem.store = alloca { i64, i32 }
  %coerce = alloca { i32, i32, i32 }, align 4
  %1 = call { i64, i32 } @callee()
  store { i64, i32 } %1, { i64, i32 }* %coerce.mem.store
  %2 = bitcast { i64, i32 }* %coerce.mem.store to i8*
  %3 = bitcast { i32, i32, i32 }* %coerce to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %3, i8* align 1 %2, i64 12, i1 false)
  %4 = load { i32, i32, i32 }* %coerce, align 4
  store { i32, i32, i32 } %4, { i32, i32, i32 }* %coerce1, align 4
  %5 = bitcast { i64, i32 }* %coerce.mem.load to i8*
  %6 = bitcast { i32, i32, i32 }* %coerce1 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %5, i8* align 1 %6, i64 12, i1 false)
  %7 = load { i64, i32 }* %coerce.mem.load
  ret { i64, i32 } %7
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #0

attributes #0 = { argmemonly nounwind }
