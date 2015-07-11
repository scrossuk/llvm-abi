; ABI: x86_64-none-linux-gnu
; FUNCTION-TYPE: {ptr, ptr} ()

declare { i8*, i8* } @callee()

define { i8*, i8* } @caller() {
  %agg.tmp = alloca { i8*, i8* }, align 8
  %1 = call { i8*, i8* } @callee()
  %2 = getelementptr { i8*, i8* }* %agg.tmp, i32 0, i32 0
  %3 = extractvalue { i8*, i8* } %1, 0
  store i8* %3, i8** %2
  %4 = getelementptr { i8*, i8* }* %agg.tmp, i32 0, i32 1
  %5 = extractvalue { i8*, i8* } %1, 1
  store i8* %5, i8** %4
  %6 = load { i8*, i8* }* %agg.tmp
  ret { i8*, i8* } %6
}
