; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void (int, ...(union { longdouble, char, long }))

declare void @callee(i32, ...)

define void @caller(i32, x86_fp80* byval align 4) {
  %indirect.arg.mem = alloca x86_fp80, align 4
  %3 = load x86_fp80* %1, align 4
  store x86_fp80 %3, x86_fp80* %indirect.arg.mem, align 4
  call void (i32, ...)* @callee(i32 %0, x86_fp80* byval align 4 %indirect.arg.mem)
  ret void
}
