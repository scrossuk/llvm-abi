; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void ({ longdouble })

declare void @callee({ x86_fp80 }* byval align 4)

define void @caller({ x86_fp80 }* byval align 4) {
  %indirect.arg.mem = alloca { x86_fp80 }, align 4
  %2 = load { x86_fp80 }* %0, align 4
  store { x86_fp80 } %2, { x86_fp80 }* %indirect.arg.mem, align 4
  call void @callee({ x86_fp80 }* byval align 4 %indirect.arg.mem)
  ret void
}
