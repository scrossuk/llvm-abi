; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void ([1 x {double, int }])

declare void @callee([1 x { double, i32 }]* byval align 4)

define void @caller([1 x { double, i32 }]* byval align 4) {
  %indirect.arg.mem = alloca [1 x { double, i32 }], align 4
  %2 = load [1 x { double, i32 }]* %0, align 4
  store [1 x { double, i32 }] %2, [1 x { double, i32 }]* %indirect.arg.mem, align 4
  call void @callee([1 x { double, i32 }]* byval align 4 %indirect.arg.mem)
  ret void
}
