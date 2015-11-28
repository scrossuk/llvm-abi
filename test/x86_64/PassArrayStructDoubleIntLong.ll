; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void ([1 x {double, int, long }])

declare void @callee([1 x { double, i32, i64 }]* byval align 8)

define void @caller([1 x { double, i32, i64 }]* byval align 8) {
  %indirect.arg.mem = alloca [1 x { double, i32, i64 }], align 8
  %2 = load [1 x { double, i32, i64 }]* %0, align 8
  store [1 x { double, i32, i64 }] %2, [1 x { double, i32, i64 }]* %indirect.arg.mem, align 8
  call void @callee([1 x { double, i32, i64 }]* byval align 8 %indirect.arg.mem)
  ret void
}
