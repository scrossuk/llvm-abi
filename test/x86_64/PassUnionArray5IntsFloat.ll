; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void (union{ [5 x int], float })

declare void @callee([5 x i32]* byval align 16)

define void @caller([5 x i32]* byval align 16) {
  %indirect.arg.mem = alloca [5 x i32], align 16
  %2 = load [5 x i32]* %0, align 16
  store [5 x i32] %2, [5 x i32]* %indirect.arg.mem, align 16
  call void @callee([5 x i32]* byval align 16 %indirect.arg.mem)
  ret void
}
