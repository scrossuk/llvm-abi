; ABI: i386-apple-darwin9
; FUNCTION-TYPE: <2 x double> ()

declare <2 x i64> @callee()

define <2 x i64> @caller() {
  %coerce1 = alloca <2 x double>, align 16
  %coerce = alloca <2 x double>, align 16
  %1 = call <2 x i64> @callee()
  %2 = bitcast <2 x double>* %coerce to <2 x i64>*
  store <2 x i64> %1, <2 x i64>* %2, align 1
  %3 = load <2 x double>* %coerce, align 16
  store <2 x double> %3, <2 x double>* %coerce1, align 16
  %4 = bitcast <2 x double>* %coerce1 to <2 x i64>*
  %5 = load <2 x i64>* %4, align 1
  ret <2 x i64> %5
}
