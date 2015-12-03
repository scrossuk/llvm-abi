; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void (union NamedUnion{ int })

%NamedUnion = type { i32 }

declare void @callee(i32)

define void @caller(i32 %coerce) {
  %coerce.arg.source = alloca %NamedUnion, align 4
  %coerce.mem = alloca %NamedUnion, align 4
  %1 = bitcast %NamedUnion* %coerce.mem to i32*
  store i32 %coerce, i32* %1, align 1
  %2 = load %NamedUnion* %coerce.mem
  store %NamedUnion %2, %NamedUnion* %coerce.arg.source
  %3 = bitcast %NamedUnion* %coerce.arg.source to i32*
  %4 = load i32* %3, align 1
  call void @callee(i32 %4)
  ret void
}
