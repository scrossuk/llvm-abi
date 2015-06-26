; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void ([1 x {double, int }])

declare void @callee(double, i32)

define void @caller(double %coerce0, i32 %coerce1) {
  %coerce.arg.source = alloca [1 x { double, i32 }], align 16
  %coerce.mem = alloca [1 x { double, i32 }], align 16
  %1 = bitcast [1 x { double, i32 }]* %coerce.mem to { double, i32 }*
  %2 = getelementptr { double, i32 }* %1, i32 0, i32 0
  store double %coerce0, double* %2
  %3 = getelementptr { double, i32 }* %1, i32 0, i32 1
  store i32 %coerce1, i32* %3
  %4 = load [1 x { double, i32 }]* %coerce.mem
  store [1 x { double, i32 }] %4, [1 x { double, i32 }]* %coerce.arg.source
  %5 = bitcast [1 x { double, i32 }]* %coerce.arg.source to { double, i32 }*
  %6 = getelementptr { double, i32 }* %5, i32 0, i32 0
  %7 = load double* %6, align 1
  %8 = getelementptr { double, i32 }* %5, i32 0, i32 1
  %9 = load i32* %8, align 1
  call void @callee(double %7, i32 %9)
  ret void
}
