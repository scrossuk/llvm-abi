; ABI: i386-apple-darwin9
; FUNCTION-TYPE: {<1 x longlong>} ()

declare void @callee({ <1 x i64> }* noalias sret)

define void @caller({ <1 x i64> }* noalias sret %agg.result) {
  %1 = alloca { <1 x i64> }, align 8
  call void @callee({ <1 x i64> }* noalias sret %1)
  %2 = load { <1 x i64> }* %1
  store { <1 x i64> } %2, { <1 x i64> }* %agg.result
  ret void
}
