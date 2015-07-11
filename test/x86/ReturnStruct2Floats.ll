; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: {float, float} ()

declare void @callee({ float, float }* noalias sret)

define void @caller({ float, float }* noalias sret %agg.result) {
  %1 = alloca { float, float }, align 4
  call void @callee({ float, float }* noalias sret %1)
  %2 = load { float, float }* %1
  store { float, float } %2, { float, float }* %agg.result
  ret void
}
