; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void ({ longdouble })
; 
; Check byval alignment.

declare void @callee({ x86_fp80 }* byval align 16)

define void @caller({ x86_fp80 }* byval align 16) {
  %indirect.arg.mem = alloca { x86_fp80 }, align 16
  %2 = load { x86_fp80 }* %0
  store { x86_fp80 } %2, { x86_fp80 }* %indirect.arg.mem
  call void @callee({ x86_fp80 }* byval align 16 %indirect.arg.mem)
  ret void
}
