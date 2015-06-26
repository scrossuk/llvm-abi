; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void ({ int, int, int, int, int })

declare void @callee(i8*)

define void @caller(i8*) {
entry:
  TODO
  call void @callee(i8* %1)
  ret void
}
