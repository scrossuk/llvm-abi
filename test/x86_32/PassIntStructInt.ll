; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void (int, { int })

declare void @callee(i32, i32)

define void @caller(i32, i32) {
  %expand.source.arg = alloca { i32 }, align 4
  %expand.dest.arg = alloca { i32 }, align 4
  %3 = getelementptr { i32 }* %expand.dest.arg, i32 0, i32 0
  store i32 %1, i32* %3, align 4
  %4 = load { i32 }* %expand.dest.arg, align 4
  store { i32 } %4, { i32 }* %expand.source.arg, align 4
  %5 = getelementptr { i32 }* %expand.source.arg, i32 0, i32 0
  %6 = load i32* %5, align 4
  call void @callee(i32 %0, i32 %6)
  ret void
}
