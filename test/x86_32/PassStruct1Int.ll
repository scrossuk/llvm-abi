; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void ({ int })

declare void @callee(i32)

define void @caller(i32) {
  %expand.source.arg = alloca { i32 }, align 4
  %expand.dest.arg = alloca { i32 }, align 4
  %2 = getelementptr { i32 }* %expand.dest.arg, i32 0, i32 0
  store i32 %0, i32* %2, align 4
  %3 = load { i32 }* %expand.dest.arg, align 4
  store { i32 } %3, { i32 }* %expand.source.arg, align 4
  %4 = getelementptr { i32 }* %expand.source.arg, i32 0, i32 0
  %5 = load i32* %4, align 4
  call void @callee(i32 %5)
  ret void
}
