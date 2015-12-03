; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void (struct NamedStruct{ int, int })

%NamedStruct = type { i32, i32 }

declare void @callee(i64)

define void @caller(i64 %coerce) {
  %coerce.arg.source = alloca %NamedStruct, align 4
  %coerce.mem = alloca %NamedStruct, align 8
  %1 = bitcast %NamedStruct* %coerce.mem to i64*
  store i64 %coerce, i64* %1, align 1
  %2 = load %NamedStruct* %coerce.mem
  store %NamedStruct %2, %NamedStruct* %coerce.arg.source
  %3 = bitcast %NamedStruct* %coerce.arg.source to i64*
  %4 = load i64* %3, align 1
  call void @callee(i64 %4)
  ret void
}
