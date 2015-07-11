; ABI: i386-apple-darwin9
; FUNCTION-TYPE: {<2 x longlong>} ()

declare void @callee({ <2 x i64> }* noalias sret)

define void @caller({ <2 x i64> }* noalias sret %agg.result) {
  %1 = alloca { <2 x i64> }, align 16
  call void @callee({ <2 x i64> }* noalias sret %1)
  %2 = load { <2 x i64> }* %1
  store { <2 x i64> } %2, { <2 x i64> }* %agg.result
  ret void
}
