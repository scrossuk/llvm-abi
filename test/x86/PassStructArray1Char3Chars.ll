; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void ({[8 x char], char, char, char})

declare void @callee({ [8 x i8], i8, i8, i8 }* byval align 4)

define void @caller({ [8 x i8], i8, i8, i8 }* byval align 4) {
  %indirect.arg.mem = alloca { [8 x i8], i8, i8, i8 }, align 4
  %2 = load { [8 x i8], i8, i8, i8 }* %0, align 4
  store { [8 x i8], i8, i8, i8 } %2, { [8 x i8], i8, i8, i8 }* %indirect.arg.mem, align 4
  call void @callee({ [8 x i8], i8, i8, i8 }* byval align 4 %indirect.arg.mem)
  ret void
}
