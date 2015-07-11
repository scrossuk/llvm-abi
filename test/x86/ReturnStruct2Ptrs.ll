; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: {ptr, ptr} ()

declare void @callee({ i8*, i8* }* noalias sret)

define void @caller({ i8*, i8* }* noalias sret %agg.result) {
  %1 = alloca { i8*, i8* }, align 4
  call void @callee({ i8*, i8* }* noalias sret %1)
  %2 = load { i8*, i8* }* %1
  store { i8*, i8* } %2, { i8*, i8* }* %agg.result
  ret void
}
