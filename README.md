# bitope
Bit-map Operatin for Windows .bmp

bitope.pdf:	処理説明

bitope.h:	bitmapを操作するユーザライブラリ(ヘッダ)
bitope.cpp:	bitmapを操作するユーザライブラリ(ソース)

ex_bitope.cpp:	 bitmapを操作するサンプル
		in.bmp			out.bmp
		(original) →	(original)(X_mirror)
									(Y_mirror)(NOT)				外枠・クロス線あり

ex_base64.cpp:	 bit列を操作するサンプル
	source text →	Base64encode/decode
	Base64 text →  Base64decode/encode
