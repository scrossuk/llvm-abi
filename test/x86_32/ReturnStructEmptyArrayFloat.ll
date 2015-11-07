; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: {[0 x char], float} ()

declare void @callee({ [0 x i8], float }* noalias sret)

define void @caller({ [0 x i8], float }* noalias sret %agg.result) {
  %1 = alloca { [0 x i8], float }, align 4
  call void @callee({ [0 x i8], float }* noalias sret %1)
  %2 = load { [0 x i8], float }* %1
  store { [0 x i8], float } %2, { [0 x i8], float }* %agg.result
  ret void
}
