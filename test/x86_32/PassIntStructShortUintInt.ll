; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void (int, {short, uint, int})

declare void @callee(i32, { i16, i32, i32 }* byval align 4)

define void @caller(i32, { i16, i32, i32 }* byval align 4) {
  %indirect.arg.mem = alloca { i16, i32, i32 }, align 4
  %3 = load { i16, i32, i32 }* %1, align 4
  store { i16, i32, i32 } %3, { i16, i32, i32 }* %indirect.arg.mem, align 4
  call void @callee(i32 %0, { i16, i32, i32 }* byval align 4 %indirect.arg.mem)
  ret void
}
