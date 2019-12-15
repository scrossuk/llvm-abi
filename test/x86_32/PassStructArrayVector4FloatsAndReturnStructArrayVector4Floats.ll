; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: {[1 x <4 x float>]} ({[1 x <4 x float>]})

declare void @callee({ [1 x <4 x float>] }* noalias sret, { [1 x <4 x float>] }* byval align 4)

define void @caller({ [1 x <4 x float>] }* noalias sret %agg.result, { [1 x <4 x float>] }* byval align 4) {
  %indirect.arg.mem = alloca { [1 x <4 x float>] }, align 16
  %2 = alloca { [1 x <4 x float>] }, align 16
  %coerce = alloca { [1 x <4 x float>] }, align 16
  %3 = bitcast { [1 x <4 x float>] }* %coerce to i8*
  %4 = bitcast { [1 x <4 x float>] }* %0 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 4 %3, i8* align 4 %4, i32 16, i1 false)
  %5 = load { [1 x <4 x float>] }* %coerce, align 16
  store { [1 x <4 x float>] } %5, { [1 x <4 x float>] }* %indirect.arg.mem, align 16
  call void @callee({ [1 x <4 x float>] }* noalias sret %2, { [1 x <4 x float>] }* byval align 4 %indirect.arg.mem)
  %6 = load { [1 x <4 x float>] }* %2
  store { [1 x <4 x float>] } %6, { [1 x <4 x float>] }* %agg.result
  ret void
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture, i8* nocapture readonly, i32, i32, i1) #0

attributes #0 = { argmemonly nounwind }
