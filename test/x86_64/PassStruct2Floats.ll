; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: void ({ float, float })

declare void @callee(<2 x float>)

define void @caller(<2 x float> %coerce) {
  %coerce.arg.source = alloca { float, float }, align 4
  %coerce.mem = alloca { float, float }, align 4
  %1 = bitcast { float, float }* %coerce.mem to <2 x float>*
  store <2 x float> %coerce, <2 x float>* %1, align 1
  %2 = load { float, float }* %coerce.mem
  store { float, float } %2, { float, float }* %coerce.arg.source
  %3 = bitcast { float, float }* %coerce.arg.source to <2 x float>*
  %4 = load <2 x float>* %3, align 1
  call void @callee(<2 x float> %4)
  ret void
}
