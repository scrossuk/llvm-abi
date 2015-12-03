; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void (struct NamedStruct{ int })

%NamedStruct = type { i32 }

declare void @callee(i32)

define void @caller(i32) {
  %expand.source.arg = alloca %NamedStruct, align 4
  %expand.dest.arg = alloca %NamedStruct, align 4
  %2 = getelementptr %NamedStruct* %expand.dest.arg, i32 0, i32 0
  store i32 %0, i32* %2, align 4
  %3 = load %NamedStruct* %expand.dest.arg, align 4
  store %NamedStruct %3, %NamedStruct* %expand.source.arg, align 4
  %4 = getelementptr %NamedStruct* %expand.source.arg, i32 0, i32 0
  %5 = load i32* %4, align 4
  call void @callee(i32 %5)
  ret void
}
