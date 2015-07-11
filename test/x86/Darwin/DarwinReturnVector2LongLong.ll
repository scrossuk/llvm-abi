; ABI: i386-apple-darwin9
; FUNCTION-TYPE: <2 x longlong> ()

declare <2 x i64> @callee()

define <2 x i64> @caller() {
  %1 = call <2 x i64> @callee()
  ret <2 x i64> %1
}
