; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void (struct NamedStruct{ ptr, int })

%NamedStruct = type { i8*, i32 }

declare void @callee(i8*, i32)

define void @caller(i8* %coerce0, i32 %coerce1) {
  %coerce.arg.source = alloca %NamedStruct, align 8
  %coerce.mem = alloca %NamedStruct, align 8
  %1 = getelementptr %NamedStruct* %coerce.mem, i32 0, i32 0
  store i8* %coerce0, i8** %1
  %2 = getelementptr %NamedStruct* %coerce.mem, i32 0, i32 1
  store i32 %coerce1, i32* %2
  %3 = load %NamedStruct* %coerce.mem
  store %NamedStruct %3, %NamedStruct* %coerce.arg.source
  %4 = getelementptr %NamedStruct* %coerce.arg.source, i32 0, i32 0
  %5 = load i8** %4, align 1
  %6 = getelementptr %NamedStruct* %coerce.arg.source, i32 0, i32 1
  %7 = load i32* %6, align 1
  call void @callee(i8* %5, i32 %7)
  ret void
}
