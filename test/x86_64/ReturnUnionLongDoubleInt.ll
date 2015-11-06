; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: union{longdouble, int} ()

declare void @callee({ x86_fp80 }* noalias sret)

define void @caller({ x86_fp80 }* noalias sret %agg.result) {
  %1 = alloca { x86_fp80 }, align 16
  call void @callee({ x86_fp80 }* noalias sret %1)
  %2 = load { x86_fp80 }* %1
  store { x86_fp80 } %2, { x86_fp80 }* %agg.result
  ret void
}
