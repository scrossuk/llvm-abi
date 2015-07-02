; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void (int, ...(<8 x float>))

declare void @callee(i32, ...)

define void @caller(i32, <8 x float>* byval align 32) {
  %indirect.arg.mem = alloca <8 x float>, align 32
  %3 = load <8 x float>* %1, align 32
  store <8 x float> %3, <8 x float>* %indirect.arg.mem, align 32
  call void (i32, ...)* @callee(i32 %0, <8 x float>* byval align 32 %indirect.arg.mem)
  ret void
}
