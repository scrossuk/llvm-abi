; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: {[3 x longlong]} (int, int, int, int, {[2 x longlong]}, int)

declare void @callee({ [3 x i64] }* noalias sret, i32, i32, i32, i32, { [2 x i64] }* byval align 4, i32)

define void @caller({ [3 x i64] }* noalias sret %agg.result, i32, i32, i32, i32, { [2 x i64] }* byval align 4, i32) {
  %indirect.arg.mem = alloca { [2 x i64] }, align 4
  %7 = alloca { [3 x i64] }, align 4
  %8 = load { [2 x i64] }* %4, align 4
  store { [2 x i64] } %8, { [2 x i64] }* %indirect.arg.mem, align 4
  call void @callee({ [3 x i64] }* noalias sret %7, i32 %0, i32 %1, i32 %2, i32 %3, { [2 x i64] }* byval align 4 %indirect.arg.mem, i32 %5)
  %9 = load { [3 x i64] }* %7
  store { [3 x i64] } %9, { [3 x i64] }* %agg.result
  ret void
}
