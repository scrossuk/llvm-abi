; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: {<8 x float>} ({<8 x float>})

declare void @callee({ <8 x float> }* noalias sret, { <8 x float> }* byval align 32)

define void @caller({ <8 x float> }* noalias sret %agg.result, { <8 x float> }* byval align 32) {
  %indirect.arg.mem = alloca { <8 x float> }, align 32
  %2 = alloca { <8 x float> }, align 32
  %3 = load { <8 x float> }* %0, align 32
  store { <8 x float> } %3, { <8 x float> }* %indirect.arg.mem, align 32
  call void @callee({ <8 x float> }* noalias sret %2, { <8 x float> }* byval align 32 %indirect.arg.mem)
  %4 = load { <8 x float> }* %2
  store { <8 x float> } %4, { <8 x float> }* %agg.result
  ret void
}
