; ABI: i386-apple-darwin9
; FUNCTION-TYPE: <2 x int> ()

declare void @callee(<2 x i32>* noalias sret)

define void @caller(<2 x i32>* noalias sret %agg.result) {
  %1 = alloca <2 x i32>, align 8
  call void @callee(<2 x i32>* noalias sret %1)
  %2 = load <2 x i32>* %1
  store <2 x i32> %2, <2 x i32>* %agg.result
  ret void
}
