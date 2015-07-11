; ABI: i386-apple-darwin9
; FUNCTION-TYPE: {<1 x double>} ()

declare void @callee({ <1 x double> }* noalias sret)

define void @caller({ <1 x double> }* noalias sret %agg.result) {
  %1 = alloca { <1 x double> }, align 8
  call void @callee({ <1 x double> }* noalias sret %1)
  %2 = load { <1 x double> }* %1
  store { <1 x double> } %2, { <1 x double> }* %agg.result
  ret void
}
