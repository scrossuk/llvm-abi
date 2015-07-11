; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: {float} ()

declare void @callee({ float }* noalias sret)

define void @caller({ float }* noalias sret %agg.result) {
  %1 = alloca { float }, align 4
  call void @callee({ float }* noalias sret %1)
  %2 = load { float }* %1
  store { float } %2, { float }* %agg.result
  ret void
}
