; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void ([1 x {double, int, int }])

declare void @callee(double, i64)

define void @caller(double %coerce0, i64 %coerce1) {
  %coerce.arg.source = alloca [1 x { double, i32, i32 }], align 8
  %coerce.mem = alloca [1 x { double, i32, i32 }], align 8
  %1 = bitcast [1 x { double, i32, i32 }]* %coerce.mem to { double, i64 }*
  %2 = getelementptr { double, i64 }* %1, i32 0, i32 0
  store double %coerce0, double* %2
  %3 = getelementptr { double, i64 }* %1, i32 0, i32 1
  store i64 %coerce1, i64* %3
  %4 = load [1 x { double, i32, i32 }]* %coerce.mem
  store [1 x { double, i32, i32 }] %4, [1 x { double, i32, i32 }]* %coerce.arg.source
  %5 = bitcast [1 x { double, i32, i32 }]* %coerce.arg.source to { double, i64 }*
  %6 = getelementptr { double, i64 }* %5, i32 0, i32 0
  %7 = load double* %6, align 1
  %8 = getelementptr { double, i64 }* %5, i32 0, i32 1
  %9 = load i64* %8, align 1
  call void @callee(double %7, i64 %9)
  ret void
}
