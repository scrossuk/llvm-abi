; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void (union{ [5 x int], float })

declare void @callee({ [5 x i32] }* byval align 8)

define void @caller({ [5 x i32] }* byval align 8) {
  %indirect.arg.mem = alloca { [5 x i32] }, align 8
  %2 = load { [5 x i32] }* %0, align 8
  store { [5 x i32] } %2, { [5 x i32] }* %indirect.arg.mem, align 8
  call void @callee({ [5 x i32] }* byval align 8 %indirect.arg.mem)
  ret void
}
