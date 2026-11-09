# bitope
Bit-map Operatin for Windows .bmp

bitope.pdf:	処理説明

bitope.h:	bitmapを操作するユーザライブラリ(ヘッダ)<br>
bitope.cpp:	bitmapを操作するユーザライブラリ(ソース)<br>

- ex_bitope.cpp:	 bitmapを操作するサンプル<br>
		in.bmp → out.bmp<br>
		(original) →	(original)(X_mirror)
						(Y_mirror)(NOT)
						外枠・クロス線あり

- ex_base64.cpp:	 bit列を操作するサンプル<br>
	source text →	Base64encode/decode<br>
	Base64 text →  Base64decode/encode
