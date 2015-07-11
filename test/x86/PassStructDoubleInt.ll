; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void ({ double, int })

declare void @callee(double, i32)

define void @caller(double, i32) {
  %expand.source.arg = alloca { double, i32 }, align 4
  %expand.dest.arg = alloca { double, i32 }, align 4
  %3 = getelementptr { double, i32 }* %expand.dest.arg, i32 0, i32 0
  store double %0, double* %3, align 4
  %4 = getelementptr { double, i32 }* %expand.dest.arg, i32 0, i32 1
  store i32 %1, i32* %4, align 4
  %5 = load { double, i32 }* %expand.dest.arg, align 4
  store { double, i32 } %5, { double, i32 }* %expand.source.arg, align 4
  %6 = getelementptr { double, i32 }* %expand.source.arg, i32 0, i32 0
  %7 = load double* %6, align 4
  %8 = getelementptr { double, i32 }* %expand.source.arg, i32 0, i32 1
  %9 = load i32* %8, align 4
  call void @callee(double %7, i32 %9)
  ret void
}
