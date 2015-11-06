; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void (union{ double, int })

declare void @callee({ double }* byval align 4)

define void @caller({ double }* byval align 4) {
  %indirect.arg.mem = alloca { double }, align 4
  %2 = load { double }* %0, align 4
  store { double } %2, { double }* %indirect.arg.mem, align 4
  call void @callee({ double }* byval align 4 %indirect.arg.mem)
  ret void
}
