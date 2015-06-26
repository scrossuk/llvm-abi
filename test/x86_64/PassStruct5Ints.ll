; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void ({ int, int, int, int, int })

declare void @callee({ i32, i32, i32, i32, i32 }* byval align 8)

define void @caller({ i32, i32, i32, i32, i32 }* byval align 8) {
  %indirect.arg.mem = alloca { i32, i32, i32, i32, i32 }, align 8
  %2 = load { i32, i32, i32, i32, i32 }* %0
  store { i32, i32, i32, i32, i32 } %2, { i32, i32, i32, i32, i32 }* %indirect.arg.mem
  call void @callee({ i32, i32, i32, i32, i32 }* byval align 8 %indirect.arg.mem)
  ret void
}
