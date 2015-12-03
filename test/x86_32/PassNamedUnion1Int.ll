; ABI: i386-none-linux-gnu
; FUNCTION-TYPE: void (union NamedUnion{ int })

%NamedUnion = type { i32 }

declare void @callee(%NamedUnion* byval align 4)

define void @caller(%NamedUnion* byval align 4) {
  %indirect.arg.mem = alloca %NamedUnion, align 4
  %2 = load %NamedUnion* %0, align 4
  store %NamedUnion %2, %NamedUnion* %indirect.arg.mem, align 4
  call void @callee(%NamedUnion* byval align 4 %indirect.arg.mem)
  ret void
}
