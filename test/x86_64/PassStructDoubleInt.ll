; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void ({ double, int })

declare void @callee(double, i32)

define void @caller(double %coerce0, i32 %coerce1) {
  %coerce.arg.source = alloca { double, i32 }, align 8
  %coerce.mem = alloca { double, i32 }, align 8
  %1 = getelementptr { double, i32 }* %coerce.mem, i32 0, i32 0
  store double %coerce0, double* %1
  %2 = getelementptr { double, i32 }* %coerce.mem, i32 0, i32 1
  store i32 %coerce1, i32* %2
  %3 = load { double, i32 }* %coerce.mem
  store { double, i32 } %3, { double, i32 }* %coerce.arg.source
  %4 = getelementptr { double, i32 }* %coerce.arg.source, i32 0, i32 0
  %5 = load double* %4, align 1
  %6 = getelementptr { double, i32 }* %coerce.arg.source, i32 0, i32 1
  %7 = load i32* %6, align 1
  call void @callee(double %5, i32 %7)
  ret void
}
