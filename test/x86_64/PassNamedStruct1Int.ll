; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void (struct NamedStruct{ int })

%NamedStruct = type { i32 }

declare void @callee(i32)

define void @caller(i32 %coerce) {
  %coerce.arg.source = alloca %NamedStruct, align 4
  %coerce.mem = alloca %NamedStruct, align 4
  %coerce.dive = getelementptr %NamedStruct* %coerce.mem, i32 0, i32 0
  store i32 %coerce, i32* %coerce.dive
  %1 = load %NamedStruct* %coerce.mem
  store %NamedStruct %1, %NamedStruct* %coerce.arg.source
  %coerce.dive1 = getelementptr %NamedStruct* %coerce.arg.source, i32 0, i32 0
  %2 = load i32* %coerce.dive1
  call void @callee(i32 %2)
  ret void
}
