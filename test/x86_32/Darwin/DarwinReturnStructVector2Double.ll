; ABI: i386-apple-darwin9
; FUNCTION-TYPE: {<2 x double>} ()

declare void @callee({ <2 x double> }* noalias sret)

define void @caller({ <2 x double> }* noalias sret %agg.result) {
  %1 = alloca { <2 x double> }, align 16
  call void @callee({ <2 x double> }* noalias sret %1)
  %2 = load { <2 x double> }* %1
  store { <2 x double> } %2, { <2 x double> }* %agg.result
  ret void
}
