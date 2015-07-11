; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void ({ int, int, int, int, int })

declare void @callee({ i32, i32, i32, i32, i32 }* byval align 4)

define void @caller({ i32, i32, i32, i32, i32 }* byval align 4) {
  %indirect.arg.mem = alloca { i32, i32, i32, i32, i32 }, align 4
  %2 = load { i32, i32, i32, i32, i32 }* %0, align 4
  store { i32, i32, i32, i32, i32 } %2, { i32, i32, i32, i32, i32 }* %indirect.arg.mem, align 4
  call void @callee({ i32, i32, i32, i32, i32 }* byval align 4 %indirect.arg.mem)
  ret void
}
