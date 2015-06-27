; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: {ptr, ptr} ()

declare { i8*, i8* } @callee()

define { i8*, i8* } @caller() {
  %coerce1 = alloca { i8*, i8* }, align 8
  %coerce = alloca { i8*, i8* }, align 8
  %1 = call { i8*, i8* } @callee()
  store { i8*, i8* } %1, { i8*, i8* }* %coerce
  %2 = load { i8*, i8* }* %coerce
  store { i8*, i8* } %2, { i8*, i8* }* %coerce1
  %3 = load { i8*, i8* }* %coerce1
  ret { i8*, i8* } %3
}
