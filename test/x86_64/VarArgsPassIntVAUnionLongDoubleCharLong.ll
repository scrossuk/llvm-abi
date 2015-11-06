; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void (int, ...(union { longdouble, char, long }))

declare void @callee(i32, ...)

define void @caller(i32, { x86_fp80 }* byval align 16) {
  %indirect.arg.mem = alloca { x86_fp80 }, align 16
  %3 = load { x86_fp80 }* %1, align 16
  store { x86_fp80 } %3, { x86_fp80 }* %indirect.arg.mem, align 16
  call void (i32, ...)* @callee(i32 %0, { x86_fp80 }* byval align 16 %indirect.arg.mem)
  ret void
}
