static char sccsid[]=
"@(#)$Workfile: bitope.cpp $$Revision: 12 $$Date: 25/02/04 19:13 $$NoKeywords: $";
/*
*****************************************************************************
*
*    bitope.c:	ＢＩＴ－ＭＡＰオペレーション
*
*
*    特記事項：
*        ・bit単位の転送処理をサポートします。
*        ・モノクロパターン(DEPTH=1)のみ対応します。
*		 ・メモリ上では黒に対応するビットをon('1')とします
*        ・次の概念をサポートします。（一部の仕様は一般的ではない）
*			bit_map_pad:	ビットマップのパディング（正数ワード化）
*			bit_map_order:	バイトの並び順（座標系に見立てます）
*			bit_byte_order:	バイト内のビットの並び順
*
*****************************************************************************
*/

#include	<stdio.h>
#include	<memory>
#include	<cstring>
#include	"bitope.h"

/* -----	define */
#define abs(x)	((x)<0?(-(x)):(x))		/* 絶対値 */

/* -----	static function */

/*
*********************************************************************
* bm_msblsb:	パディング単位のbitイメージを鏡像反転させる
*********************************************************************
*/
static void
bm_msblsb(
	bm_maxpad_t	src,	/* 元のbitイメージ */
	bm_maxpad_t*	dst,	/* 鏡像反転後のbitイメージ */
	int			bits)	/* srcの有効上位ｂｉｔ数 */
{
	unsigned int	srcpos, dstpos;	/* ビット位置 */
	int				i;

	/* -----    procedure */

	*dst = 0x0;					/* NULL */
	srcpos = 0x80000000;		/* srcのMSBからbitを取り出す */
	dstpos = 0x00000001;		/* dstのLSBからbitをセットする */

	for (i = 0;i < bits;i++, srcpos >>= 1, dstpos <<= 1) {
		if (src & srcpos) {
			(*dst) |= dstpos;
		}
	}
}

/*
**********************************************************************
*	bm_getpos:	位置ぎめをして最初のパディング単位サイズ分を取り出す
**********************************************************************
*/
static void					/* 0:データあり/-1:データ終了 */
bm_getpos(
	struct bm_image_s* dst,	/* bit-mapイメージ */
	int				  x,	/* x座標(bit単位) */
	int				  y,	/* y座標(bit単位) */
	bm_maxpad_t* pattern,		/* bitパターン(上位バイトにセットする) */
	int				   len)	/* 読み込みビット数(パディングサイズ以内) */
{
	int			pos;		/* バイト位置 */
	int			offset;		/* ビットオフセット(bit数) */
	int			eol = 0;	/* -1:end of line/0:else */
	bm_maxpad_t	nextptn;	/* 次のパディングのビットパターン */

	/* -----	procedure */

		/* 先頭ビットの存在するパディング位置(バイト単位)を算出する */
	pos = x / dst->bits * dst->bytes;	/* x方向を算出 */
	pos += (dst->iw_byte * y);		/* y方向を追加 */
	offset = x % dst->bits;	/* パディング内のオフセット */

	/* パディング単位サイズ分以内を取り出す */
	*pattern = 0x0;
	memcpy((char*)pattern, dst->image + pos, dst->bytes);
	bm_swab(pattern);	/* swap byteを変換する */
	if (x + len > dst->x + dst->w) {
		eol = -1;	/* 今回でこの行は入力終了 */
	}

	/* パディング内オフセット分を補正する */
	if (offset != 0) {
		/* イメージ上に次のパディングがあれば読み込む */
		nextptn = 0x0;
		if ((offset + len > dst->bits) && eol != -1) {
			/* 次の１パディングから不足ビットを補う */
			memcpy((char*)&nextptn, dst->image + pos + dst->bytes, dst->bytes);
			bm_swab(&nextptn);	/* swap byteを変換する */
			(*pattern) = (((*pattern) << offset)
				& ~lsb_mask[offset])	/* 上位パディングの上位 */
				|
				((nextptn >> (dst->bits - offset))
					& lsb_mask[offset]);	/* 下位パディングの下位 */
		}
		else {
			(*pattern) <<= offset;
		}
	}
	(*pattern) &= msb_mask[len];
}

/*
**********************************************************************
*	bm_setpos:	位置ぎめをして最初のパディング単位サイズ分を書き込む
**********************************************************************
*/
static void
bm_setpos(
	struct bm_image_s* dst,		/* bit-map イメージ */
	int				  x,		/* x座標(bit単位) */
	int				  y,		/* y座標(bit単位) */
	bm_maxpad_t		  pattern,	/* bitパターン(上位バイトにセットする) */
	int				  len)		/* 書き込みbit数(パディングサイズ以内) */
{
	bm_maxpad_t	currptn, nextptn;	/* 書き込み位置のbitパターン */
	bm_maxpad_t	mask;				/* patternの有効bit-mask */
	int			nextlen;			/* 次のパディングからの読み込みbit数 */
	int			offset;				/* パディング内オフセット(bit) */
	int			pos;				/* 読み込みパディング位置(byte) */

	/* -----	procedure */

		/* patternの有効ビットを現すmaskをえる */
	if (x + len > dst->x + dst->w) len = dst->x + dst->w - x;	/* lenを補正する */
	mask = msb_mask[len];

	/* 先頭ビットの存在するパディング位置(バイト単位)を算出する */
	pos = x / dst->bits * dst->bytes;	/* x方向を算出 */
	pos += (dst->iw_byte * y);		/* y方向を追加 */
	offset = x % dst->bits;		/* パディング内offset(bit) */

	if (offset == 0 && len == dst->bits) {
		currptn = 0;
	}
	else {
		/* 書き込み位置の現在の値を取り出す */
		bm_getpos(dst, x / dst->bits * dst->bits, y, &currptn, dst->bits);
	}
	if (offset + len > dst->bits) {
		/* 次の書き込み位置の現在の値を取り出す */
		nextlen = dst->bits * 2 - offset - len;
		bm_getpos(dst, (x + len) / dst->bits * dst->bits, y, &nextptn, dst->bits);
	}

	/* 書き込み位置のパディングのイメージを編集し書き込む */
	if (offset == 0) {
		currptn = (pattern & mask) | (currptn & ~mask);
		bm_swab(&currptn);	/* swap byteを変換する */
		memcpy(dst->image + pos, (char*)&currptn, dst->bytes);
	}
	else {
		currptn = (currptn & ~(mask >> offset))
			|
			((pattern & mask) >> offset);
		bm_swab(&currptn);	/* swap byteを変換する */
		memcpy(dst->image + pos, (char*)&currptn, dst->bytes);
		if (offset + len > dst->bits) {
			/* 次の書き込み位置のイメージを編集し書き込む */
			nextptn = (nextptn & lsb_mask[nextlen])
						|
					  ((pattern & mask) << (dst->bits - offset));
			bm_swab(&nextptn);	/* swap byteを変換する */
			memcpy(dst->image + pos + dst->bytes, (char*)&nextptn, dst->bytes);
		}
	}
}


/* -----	global function */

/*
*********************************************************************
* bm_swab:	swap byteを実行する
*********************************************************************
*/
void
bm_swab(		/* swab */
	bm_maxpad_t* word)	/* 対象の4バイト*/
{
	char	src[4];	/* 変換用の4バイト*/

	/* ----    procdure */

	memcpy(src, word, 4);	/* swab */
	*(((char*)word) + 3) = *(src + 0);
	*(((char*)word) + 2) = *(src + 1);
	*(((char*)word) + 1) = *(src + 2);
	*(((char*)word) + 0) = *(src + 3);
}

/*
*********************************************************************
* bm_create:	bit-mapイメージを作成する
*********************************************************************
*/
struct bm_image_s *
bm_create(
int			width,		/* イメージの幅(bit) */
int			height,		/* イメージの高さ(bit) */
bm_pad_t	bm_pad,		/* パディングの単位 */
bm_order_t	bm_order,	/* バイトオーダーを現す座標系 */
bm_byte_t	bm_byte,	/* バイト内のビットオーダー */
bm_onoff_t	bm_onoff,	/* 黒がon('1')かoff('0')か */
char		init)		/* 初期化イメージ(8-bitパターン)(MSBFirst) */
{
	struct bm_image_s *bm;	/* 作成されたbit-map */

/* -----    procedure */

	/* bit-map領域を確保し、定義する */
	if((bm=(struct bm_image_s *)malloc(sizeof(struct bm_image_s)))==NULL){
		goto exit_procedure;
	}
	memset(bm,0x00,sizeof(struct bm_image_s));
	bm->bm_pad	 = bm_pad;
	bm->bm_order = bm_order;
	bm->bm_byte  = bm_byte;
	bm->bm_onoff = bm_onoff;
	bm->w = width;
	bm->h = height;
	bm->ih = bm->h;
	bm->bytes = BM_PADBYTES(bm->bm_pad);	/* パディングのbyte数 */
	bm->bits = BM_PADBITS(bm->bm_pad);		/* パディングのbit数 */
	/* iwとパディングサイズで行のバイト数を算出する */
	bm->iw = (bm->w/ BM_PADBITS(bm->bm_pad) + (bm->w% BM_PADBITS(bm->bm_pad) ==0?0:1))* BM_PADBITS(bm->bm_pad);
	bm->iw_byte = bm->iw/ BM_PADBITS(bm->bm_pad) * BM_PADBYTES(bm->bm_pad);

	/* 正規化したメモリ割当サイズを算出し、メモリを確保する */
	bm->isize = bm->iw_byte * height;
	if((bm->image = (char *)malloc(bm->isize))==NULL){
		free(bm);
		bm = NULL;
		goto exit_procedure;
	}

	/* bit-mapを初期化イメージにする */
	memset(bm->image,init, bm->isize);

exit_procedure:
	return(bm);
}

/*
*********************************************************************
* bm_load:	bit-mapファイルをメモリに読み込む
*********************************************************************
*/
struct bm_image_s *
bm_load(
char		*bm_file,	/* bit-mapファイル名 */
bm_ftype_t	ftype)		/* bit-mapファイルの形式 */
{
	FILE 	*bitmapfp;		/* bit-mapファイルへのポインタ */
	char	header[256];	/* ヘッダバッファ */
	struct bm_image_s *bm;	/* bit-map */
	int		width,height;	/* bit-mapの幅と高さ(bit) */

/* -----	procedure */

	bitmapfp = NULL;
	bm = NULL;

	if(ftype!=BM_PAINT/* ペイントブラシ */){
		goto exit_procedure;
	}

	/* bit-mapファイルをオープンする */
	if(fopen_s(&bitmapfp, bm_file, "rb") != 0){
		goto exit_procedure;
	}

	/* -----	for ペイントブラシ(MS-Windows) */
	/* ヘッダ情報(62バイト)を入力する */
	memset(header,0x00,sizeof(header));
	if(fread(header, 62, 1, bitmapfp) != 1 ) {
		goto exit_procedure;
	}
	if(strncmp(header, "BM", 2) != NULL) {
		goto exit_procedure;
	}


	/* bit-mapのサイズを取り出しメモリを確保する */
	memcpy((char *)&width,header+0x12,4);	/* swab */
	memcpy((char *)&height,header+0x16,4);	/* swab */
	if((bm = bm_create(width,height,BM_PAD32,BM_1ST,BM_MSBFirst,BM_OFF,0x00))==NULL){
		goto exit_procedure;
	}

	/* メモリイメージにヘッダをセットする */
	bm->hsize  = 62;
	if((bm->header = (char *)malloc(bm->hsize))==NULL){
		free(bm);
		bm = NULL;
		goto exit_procedure;
	}
	memcpy(bm->header,header,bm->hsize);

	/* bitイメージを入力する(パディング単位は一致している) */
	if(fread(bm->image, bm->isize, 1, bitmapfp) != 1){
		free(bm);
		bm = NULL;
		goto exit_procedure;
	}

exit_procedure:
	if(bitmapfp!=NULL)	fclose(bitmapfp);
	return(bm);
}
/*
*********************************************************************
* bm_mkheader:	bit-mapヘッダを作成する
*********************************************************************
*/
char*
bm_mkheader(
	char* curr, /*現在のヘッダ(NULL:メモリ確保する)*/
	int hsize,	/* ヘッダサイズ(byte)(62 for .bmp) */
	int	isize,	/* bit-mapイメージのサイズ(byte) */
	int	w,		/* bit-mapの幅(bit) */
	int	h)		/* bit-mapの高さ(bit) */
{
	char* header;	/* bit-mapヘッダ */
	int	  fsize;	/* ファイルサイズ */

	/* -----	procedure */

	fsize = isize + hsize;
	if (curr == NULL) {
		header = (char*)malloc(hsize < 62 ? 62 : hsize);
		memset(header, 0x00, hsize);
	}
	else {
		header = curr;
	}

	memcpy(header, (char *)"BM", 2);				/* ファイルタイプ */
	memcpy(header + 0x02, (char*)&fsize, 4);		/* ファイルサイズ */
	memcpy(header + 0x0A, (char*)"\x3E\x00\x00\x00", 4);	/* オフセット */
	memcpy(header + 0x0E, (char*)"\x28\x00\x00\x00", 4);	/* ヘッダサイズ */
	memcpy(header + 0x12, (char*)&w, 4);			/* 幅 */
	memcpy(header + 0x16, (char*)&h, 4);			/* 高さ */
	memcpy(header + 0x1A, (char*)"\x01\x00", 2);	/* プレーン数 */
	memcpy(header + 0x1C, (char*)"\x01\x00", 2);	/* ビット数 */
	memcpy(header + 0x26, (char*)"\x11\x0B\x00\x00", 4);	/* 水平方向の解像度 */
	memcpy(header + 0x2A, (char*)"\x11\x0B\x00\x00", 4);	/* 垂直方向の解像度 */
	memcpy(header + 0x2E, (char*)"\x02\x00\x00\x00", 4);	/* プレーン数 */
	memcpy(header + 0x32, (char*)"\x00\x00\x00\x00", 4);	/* ビット数 */
	memcpy(header + 0x36, (char*)"\x00\x00\x00\x00", 4);	/* 不明(カラーマスク) */
	memcpy(header + 0x3A, (char*)"\xFF\xFF\xFF\x00", 4);	/* 不明(カラーマスク) */

	return(header);	
}

/*
*********************************************************************
* bm_save:	bit-mapファイルに保存する
*********************************************************************
*/
int
bm_save(
struct bm_image_s *bm,		/* bit-map */
char			  *bm_file)	/* bit_mapファイル名 */
{
	FILE 	*bitmapfp;		/* bit-mapファイルへのポインタ */
	int		 stat;

/* -----	procedure */

	bitmapfp = NULL;
	stat 	 = -1;

	/* bit-mapファイルをオープンする */
	if(fopen_s(&bitmapfp, bm_file, "wb") != 0){
		goto exit_procedure;
	}

	/* ヘッダ情報を出力する */
	if(bm->header!=NULL && bm->hsize>0){
		if(fwrite(bm->header,bm->hsize,1,bitmapfp)==0){
			goto exit_procedure;
		}
	}

	/* bitイメージを出力する */
	if(fwrite(bm->image,bm->isize,1,bitmapfp)==0){
		goto exit_procedure;
	}

	stat = 0;

exit_procedure:
	if(bitmapfp!=NULL)	fclose(bitmapfp);
	return(stat);
}

/*
**********************************************************************
* bm_sethead:	bit-mapファイルの編集済のヘッダ情報をセットする
**********************************************************************
*/
void
bm_sethead(
struct bm_image_s *bm,	/* bit-map */
char	*header,		/* ヘッダ（bit-mapのファイル形式によって異なる） */
int		 size)			/* ヘッダのサイズ(byte) */
{
/* -----	procedure */

	/* ヘッダのメモリを割り当てる */
	if(bm->hsize<size){
		if(bm->header!=0) free(bm->header);
		bm->header = (char *)malloc(size);
		bm->hsize  = size;
	}else{
		bm->hsize = size;
	}
	memset(bm->header,0x00,bm->hsize);
	
	/* ヘッダの内容をセットする */
	memcpy(bm->header,header,bm->hsize);
}

/*
*********************************************************************
* bm_rop:	イメージをオペレーションに従って処理する
*			基本イメージ(オーダー:BM_4TH,黒:BM_ON)を前提とする
*			パディングサイズ，ビットオーダーは任意でよい
*********************************************************************
*/
void
bm_rop(
struct bm_image_s *dst,		/* 複写先 */
struct bm_image_s *src,		/* 複写元(0/1の場合はall 0/1を仮定する) */
bm_ope_t		   rop)		/* Raster-Operation */
{
	bm_maxpad_t	*dst_bits;	/* dstのビット列 */
	bm_maxpad_t	*src_bits;	/* srcのビット列 */
	int			size;		/* メモリサイズ(byte) */
	int			nofpad;		/* パディング数 */
	int			save_y_src,save_y_dst,save_h_dst;/* 値を保存する */
	int			h,i;

/* -----	procedure */

	/* １ライン(h=1)分のビット列のメモリを確保する */
	nofpad	= dst->w/BM_PADBITS_MAX+(dst->w%BM_PADBITS_MAX==0?0:1);
	size	= nofpad * BM_PADBYTES_MAX;
	dst_bits = (bm_maxpad_t *)malloc(size);
	src_bits = (bm_maxpad_t *)malloc(size);

	/* 値を保存しておく */
	save_y_dst = dst->y;
	if(src!=BM_SRC0 && src!=BM_SRC1) save_y_src = src->y;
	save_h_dst = dst->h;

	/* ライン数分(dst->h)実行する */
	for(dst->h=1,h=0;h<save_h_dst;h++){

		/* srcビット列を読み込む */
		memset(src_bits,0x00,size);
		if(src==BM_SRC0){
			/* all '0'を仮定する */
		}else if(src==BM_SRC1){
			/* all '1'を仮定する */
			memset(src_bits,0xff,size);
		}else{
			/* ビット列を読み込む */
			src->w = dst->w;
			src->h = dst->h;
			bm_read(src,src_bits);
		}
	
		/* dstビット列を読み込む */
		memset(dst_bits,0x00,size);
		bm_read(dst,dst_bits);
	
		/* オペレーションを実行する */
		switch((int)rop){
	  	case BM_0:		/* 0 */
			for(i=0;i<nofpad;i++){
				*(dst_bits + i) = 0;
			}
			break;
	  	case BM_1:		/* 1 */
			for(i=0;i<nofpad;i++){
				*(dst_bits + i) = 0xffffffff;
			}
			break;
	  	case BM_and:		/*  dst &  src */
			for(i=0;i<nofpad;i++){
				*(dst_bits + i) = *(dst_bits +i) & *(src_bits + i);
			}
			break;
	  	case BM_Nand:		/* ~dst &  src */
			for(i=0;i<nofpad;i++){
				*(dst_bits + i) = ~*(dst_bits +i) & *(src_bits + i);
			}
			break;
	  	case BM_andN:		/*  dst & ~src */
			for(i=0;i<nofpad;i++){
				*(dst_bits + i) = *(dst_bits +i) & ~*(src_bits + i);
			}
			break;
	  	case BM_NandN:	/* ~dst & ~src */
			for(i=0;i<nofpad;i++){
				*(dst_bits + i) = ~*(dst_bits +i) & ~*(src_bits + i);
			}
			break;
	  	case BM_or:		/*  dst |  src */
			for(i=0;i<nofpad;i++){
				*(dst_bits + i) = *(dst_bits +i) | *(src_bits + i);
			}
			break;
	  	case BM_Nor:		/* ~dst |  src */
			for(i=0;i<nofpad;i++){
				*(dst_bits + i) = ~*(dst_bits +i) | *(src_bits + i);
			}
			break;
	  	case BM_orN:		/*  dst | ~src */
			for(i=0;i<nofpad;i++){
				*(dst_bits + i) = *(dst_bits +i) | ~*(src_bits + i);
			}
			break;
	  	case BM_NorN:		/* ~dst | ~src */
			for(i=0;i<nofpad;i++){
				*(dst_bits + i) = ~*(dst_bits +i) | ~*(src_bits + i);
			}
			break;
	  	case BM_xor:		/*  dst xor  src */
			for(i=0;i<nofpad;i++){
				*(dst_bits + i) = *(dst_bits +i) ^ *(src_bits + i);
			}
			break;
	  	case BM_xorN:		/*  dst xor ~src */
			for(i=0;i<nofpad;i++){
				*(dst_bits + i) = *(dst_bits +i) ^ ~*(src_bits + i);
			}
			break;
	  	case BM_noop:		/*  dst */
				break;
	  	case BM_N:		/* ~dst */
			for(i=0;i<nofpad;i++){
				*(dst_bits + i) = ~*(dst_bits +i);
			}
			break;
	  	case BM_set:		/*  src */
			for(i=0;i<nofpad;i++){
				*(dst_bits + i) = *(src_bits + i);
			}
			break;
	  	case BM_Nset:		/* ~src */
			for(i=0;i<nofpad;i++){
				*(dst_bits + i) = ~*(src_bits + i);
			}
			break;
		}

		/* dstにビット列を書き込む */
		bm_write(dst,dst_bits);

		dst->y ++;
		if(src!=BM_SRC0 && src!=BM_SRC1) src->y++;
	}

	/* 値を復元する */
	dst->y = save_y_dst;
	dst->h = save_h_dst;
	if(src!=BM_SRC0 && src!=BM_SRC1){
		src->y = save_y_src;
		src->h = dst->h;
	}

	/* メモリを解放する */
	free(dst_bits);
	free(src_bits);
}

/*
**********************************************************************
*	bm_conv:	オーダー(bm_order,bm_byte)，白黒(bm_onoff)を変換する
*				イメージサイズ，パディングサイズは一致していること
* 				イメージサイズはパディングの整数倍であること 
**********************************************************************
*/
void
bm_conv(
struct bm_image_s *dst,		/* 変換結果 */
struct bm_image_s *src,		/* 変換もと */
bm_cnv_t		  mirror)	/* 鏡像変換オプション */
{
	int		x_hi,y_hi;		/* 読み込み方向 */
	int		getpos;			/* 読み込み位置 */
	int		x_len;			/* xの増分(src側のパディング単位とする(byte)) */
	int		y_len;			/* yの増分(１ラインの長さ(byte)) */
	int		y_lenpos;
	int		setpos,setx_len,sety_len;	/* 書き込み側の位置など */
	int		h,w,w_pad;		/* ライン,パディングカウンタ,１ラインのパディング */
	bm_maxpad_t	buf;		/* イメージバッファ */

/* -----	procedure */

	/* パディングオーダーの読み込み方向，増分を算出する */
	x_hi = ((0xf0&(int)dst->bm_order)==(0xf0&(int)src->bm_order)?0x0:0x1);
	x_hi = ((mirror&BM_X_MIRROR)?(~x_hi&0x1):x_hi);/* 鏡像反転指定なら反対にする */
	x_len = BM_PADBYTES(src->bm_pad);
	x_len = (x_hi?(-x_len):x_len);
	setx_len = abs(x_len);

	/* ラインオーダーの読み込み方向，増分を算出する */
	y_hi = ((0x0f&(int)dst->bm_order)==(0x0f&(int)src->bm_order)?0x0:0x1);
	y_hi = (mirror&BM_Y_MIRROR?(~y_hi&0x1):y_hi);	/* 鏡像反転指定なら反対にする */
	y_len = src->iw/BM_PADBITS(src->bm_pad)*BM_PADBYTES(src->bm_pad);
	w_pad = src->iw/BM_PADBITS(src->bm_pad);
	y_len = (y_hi?(-y_len):y_len);
	y_lenpos = (x_hi==y_hi?0:(y_len*2));
	sety_len = 0;

	/* 読み込み先頭位置を算出する */
	getpos = (x_hi?abs(y_len)+x_len:0);			/* xオフセットを求める */
	getpos += (y_hi?abs(y_len)*(src->ih-1):0);	/* y方向にシフトする */
	setpos = 0;

	/* 転送する */
	//for (int i = 0;i < src->isize;i++) { dst->image[i] = src->image[i]; }

	for(h=0;h<src->ih;h++,getpos+=y_lenpos,setpos+=sety_len){
		for(w=0;w<w_pad;w++,getpos+=x_len,setpos+=setx_len){
			memcpy((char*)&buf,src->image+getpos,setx_len);	/* 読みだし */
			bm_swab(&buf);	/* swap byteを変換する */
			if(mirror&BM_X_MIRROR && dst->bm_byte==src->bm_byte){
				/* ｘ方向に鏡像反転の場合は、ビットオーダーを入れ換える */
				bm_msblsb(buf,&buf,BM_PADBITS_MAX);
				if(setx_len!=BM_PADBYTES_MAX){
					buf <<= (BM_PADBITS_MAX-BM_PADBITS(src->bm_pad));
				}
			}
			/* 白黒のon/offを変換する */
			if(dst->bm_onoff != src->bm_onoff){
				buf = ~buf;
			}
			bm_swab(&buf);	/* swap byteを変換する */
			memcpy(dst->image+setpos, (char*)&buf,setx_len);	/* 書き込み */
		}
	}
}

/*
**********************************************************************
*	bm_read:	イメージからビット列を読み出す
**********************************************************************
*/
void
bm_read(
struct	bm_image_s *dst,		/* ビットマップイメージ */
bm_maxpad_t		   *pattern)	/* ビット列 */
{
	int				read_x,read_y;	/* 読み込み位置 */
	int				len,res_len;	/* １回の読み込みビット数 */
	bm_maxpad_t		buf;
	char			*p;				/* 読み込みビット列のアドレス(byte) */
	int				nx,xloop;		/* xの読み込み回数 */
	int				ny;				/* yの読み込み回数 */

/* -----	procedure */

	/* 読み込み位置を初期化する */
	read_x = dst->x;
	read_y = dst->y;

	p 		= (char *)pattern;
	len		= dst->bits;			/* パディングのビット数 */
	xloop	= dst->w/len;
	res_len = dst->w%len;			/* パディング単位の端数分 */

	/* 高さの回数読み込む */
	for(ny=0;ny<dst->h;ny++){
		/* パディングサイズ分ずつ読み込む */
		for(nx=0;nx<xloop;nx++){
			bm_getpos(dst,read_x,read_y,&buf,len);
			memcpy(p, (char*)&buf,dst->bytes);
			read_x += dst->bits;
			p += dst->bytes;
		}
		if(res_len!=0){
			/* x方向の端数分を読み込む */
			bm_getpos(dst,read_x,read_y,&buf,res_len);
			memcpy(p, (char*)&buf,dst->bytes);
			read_x += dst->bits;
			p += dst->bytes;
		}
		/* ｙ方向に読み込み位置を修正する */
		read_x = dst->x;
		read_y ++;
		if(read_y > dst->ih){
			break;
		}
	}
}

/*
**********************************************************************
*	bm_write:	ビット列をイメージに書き込む
**********************************************************************
*/
void
bm_write(
struct	bm_image_s *dst,		/* ビットマップイメージ */
bm_maxpad_t		   *pattern)	/* ビット列 */
{
	int				write_x,write_y;/* 書き込み位置 */
	int				len,res_len;	/* １回の書き込みビット数 */
	char			*p;				/* 書き込みビット列のbyteアドレス */
	bm_maxpad_t		buf;			/* 書き込みバッファ */
	int				nx,xloop;		/* xの書き込み回数 */
	int				ny;				/* yの書き込み回数 */

/* -----	procedure */

	/* 書き込み位置を初期化する */
	write_x = dst->x;
	write_y = dst->y;

	p 		= (char *)pattern;
	len		= dst->bits;			/* パディングのビット数 */
	xloop	= dst->w/len;
	res_len = dst->w%len;			/* パディング単位の端数分 */
	buf 	= 0;

	/* 高さの回数書き込む */
	for(ny=0;ny<dst->h;ny++){
		/* パディングサイズ分ずつ書き込む */
		for(nx=0;nx<xloop;nx++){
			memcpy((char*)&buf,p,dst->bytes);
			bm_setpos(dst,write_x,write_y,buf,len);
			write_x += dst->bits;
			p += dst->bytes;
		}
		if(res_len!=0){
			/* x方向の端数分を書き込む */
			memcpy((char*)&buf,p,dst->bytes);
			bm_setpos(dst,write_x,write_y,buf,res_len);
			write_x += dst->bits;
			p += dst->bytes;
		}
		/* ｙ方向に書き込み位置を修正する */
		write_x = dst->x;
		write_y ++;
		if(write_y > dst->ih){
			break;
		}
	}
}

/*
**********************************************************************
*	bm_kill:	ビットイメージのエリアを解放する
**********************************************************************
*/
void 
bm_kill(
struct bm_image_s *bm)		/* bit-mapイメージ */
{
/* -----	procedure */

	if(bm!=NULL){
		if(bm->header!=NULL)	free(bm->header);
		if(bm->image!=NULL)		free(bm->image);
		free(bm);
	}
}
