; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void (union{ double, int })

declare void @callee(i64)

define void @caller(i64 %coerce) {
  %coerce.arg.source = alloca double, align 8
  %coerce.mem = alloca double, align 8
  %1 = bitcast double* %coerce.mem to i64*
  store i64 %coerce, i64* %1, align 1
  %2 = load double* %coerce.mem
  store double %2, double* %coerce.arg.source
  %3 = bitcast double* %coerce.arg.source to i64*
  %4 = load i64* %3, align 1
  call void @callee(i64 %4)
  ret void
}
