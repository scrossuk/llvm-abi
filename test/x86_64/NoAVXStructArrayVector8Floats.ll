; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: {[1 x <8 x float>]} ({[1 x <8 x float>]})

declare void @callee({ [1 x <8 x float>] }* noalias sret, { [1 x <8 x float>] }* byval align 32)

define void @caller({ [1 x <8 x float>] }* noalias sret %agg.result, { [1 x <8 x float>] }* byval align 32) {
  %indirect.arg.mem = alloca { [1 x <8 x float>] }, align 32
  %2 = alloca { [1 x <8 x float>] }
  %3 = load { [1 x <8 x float>] }* %0
  store { [1 x <8 x float>] } %3, { [1 x <8 x float>] }* %indirect.arg.mem
  call void @callee({ [1 x <8 x float>] }* noalias sret %2, { [1 x <8 x float>] }* byval align 32 %indirect.arg.mem)
  %4 = load { [1 x <8 x float>] }* %2
  store { [1 x <8 x float>] } %4, { [1 x <8 x float>] }* %agg.result
  ret void
}
