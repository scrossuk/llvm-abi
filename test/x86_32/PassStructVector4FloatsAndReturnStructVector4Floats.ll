; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: {<4 x float>} ({<4 x float>})

declare void @callee({ <4 x float> }* noalias sret, { <4 x float> }* byval align 4)

define void @caller({ <4 x float> }* noalias sret %agg.result, { <4 x float> }* byval align 4) {
  %indirect.arg.mem = alloca { <4 x float> }, align 16
  %2 = alloca { <4 x float> }, align 16
  %coerce = alloca { <4 x float> }, align 16
  %3 = bitcast { <4 x float> }* %coerce to i8*
  %4 = bitcast { <4 x float> }* %0 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %3, i8* %4, i32 16, i32 4, i1 false)
  %5 = load { <4 x float> }* %coerce, align 16
  store { <4 x float> } %5, { <4 x float> }* %indirect.arg.mem, align 16
  call void @callee({ <4 x float> }* noalias sret %2, { <4 x float> }* byval align 4 %indirect.arg.mem)
  %6 = load { <4 x float> }* %2
  store { <4 x float> } %6, { <4 x float> }* %agg.result
  ret void
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture, i8* nocapture readonly, i32, i32, i1) #0

attributes #0 = { nounwind }
