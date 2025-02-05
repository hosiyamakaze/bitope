#pragma	comment( user,"@(#)$Workfile: bitope.h $$Revision: 8 $$Date: 25/02/04 19:13 $$NoKeywords: $")
/****************************************************************************
*
*    bitope.h:	ＢＩＴ－ＭＡＰオペレーション
*
*
*    特記事項：
*			bit単位の転送処理をサポートします。
*			モノクロパターン(1ピクセルあたりのビット数=1)のみ対応します。
*			メモリ上では黒に対応するビットをon('1')とします
*			bit_map_pad:	ビットマップのパディング（正数境界のみ）
*			bit_map_order:	バイトの並び順（座標系に見立てます）
*			bit_byte_order:	ビットの並び順（MSB Firstのみ）
*
*****************************************************************************
*/

/* -----	define */
#define		BM_PADBITS_MAX		32				/* パディングの最大サイズ(bit) */
#define		BM_PADBYTES_MAX		4				/* パディングの最大サイズ(byte) */
#define		BM_PADBITS(pad)		((int)pad)		/* パディングのサイズ(bit) */
#define		BM_PADBYTES(pad)	((int)pad/8)	/* パディングのサイズ(byte) */
#define		BM_SRC0				((struct bm_image_s *)0)	/* all 0のイメージ */
#define		BM_SRC1				((struct bm_image_s *)1)	/* all 1のイメージ */

/* -----    typedef */
/* パディングの最大単位 */
typedef unsigned long int	bm_maxpad_t;

/* bm_padの設定値（パディングの単位） */
typedef  enum {
		BM_PAD32	=	32		/* 正数境界 */
}	bm_pad_t;

/* bm_orderの設定値（バイトの並びを規定する） */
typedef enum {
		BM_1ST	=	0x00|0x01,	/* 第１象現の座標系的、原点(0,0)～(→＋)(↑＋) */
		BM_4TH	=	0x00|0x00	/* 第４象現の座標系的、原点(0.0)～(→＋)(↓＋) */
		/* 0x10はXがhi-load,0x01はyがhi-loadであることを現す */
}	bm_order_t;

/* bm_byteの設定値（ビットの並びを規定する） */
typedef enum {
		BM_MSBFirst				/* パディング単位内ではＭＳＢからセットする */
}	bm_byte_t;

/* bm_ftypeの設定値（bit-mapファイルの形式）*/
typedef enum {
		BM_PAINT			/* ペイントブラシ(MS_WINDOWS) */
}	bm_ftype_t;

/* bm_onoffの設定値(黒がビットonか否か) */
typedef enum {
		BM_ON,				/* 黒がbitをon('1')にする */
		BM_OFF				/* 黒がbitをoff('0')にする */
}	bm_onoff_t;

/* -----	cnvertoption */
typedef enum {
		BM_NULL =		0x00,	/* 何もしない */	
		BM_X_MIRROR	=	0x10,	/* ｘ方向に鏡像反転する */
		BM_Y_MIRROR	=	0x01	/* ｙ方向に鏡像反転する */
}	bm_cnv_t;

/* -----	Raster-Operation */
typedef enum {
	BM_0,		/* 0 */
	BM_1,		/* 1 */
	BM_and,		/*  dst &  src */
	BM_Nand,	/* ~dst &  src */
	BM_andN,	/*  dst & ~src */
	BM_NandN,	/* ~dst & ~src */
	BM_or,		/*  dst |  src */
	BM_Nor,		/* ~dst |  src */
	BM_orN,		/*  dst | ~src */
	BM_NorN,	/* ~dst | ~src */
	BM_xor,		/*  dst xor  src */
	BM_xorN,	/*  dst xor ~src */
	BM_noop,	/*  dst */
	BM_N,		/* ~dst */
	BM_set,		/*  src */
	BM_Nset		/* ~src */
}	bm_ope_t;

/* -----	structure */
/* １イメージを表現する構造体 */
struct bm_image_s {	
	bm_pad_t	bm_pad;		/* パディングの単位 */
	bm_order_t	bm_order;	/* 座標系 */
	bm_byte_t	bm_byte;	/* バイト内のbitの並び順 */
	bm_onoff_t	bm_onoff;	/* 黒がon('1')かoff('0')か */
	int			iw;			/* 矩形領域の幅(bit)（パディングを含む） */
	int			ih;			/* 矩形領域の高さ(bit) */
	int			x;			/* イメージへのｘオフセット(bit) */
	int			y;			/* イメージへのｙオフセット(bit) */
	int			w;			/* イメージの幅(bit)（パディングを含まない） */
	int			h;			/* イメージの高さ(bit) */
	char		*image;		/* bitイメージ */
	int			isize;		/* bitイメージのサイズ(byte)(iw_byte*ih */
	char		*header;	/* bit-mapファイルのヘッダ */
	int			hsize;		/* ヘッダのサイズ(byte) */
	int			iw_byte;	/* iwのバイト数 */
	int			bytes;		/* パディング単位のバイトBM_PADBYTES(bm_pad)数 */
	int			bits;		/* パディング単位のビットBM_BADBITS(bm_pad)数 */
};

/* -----	processor */
void			   bm_swab(		/* swap byteを実行する */
	bm_maxpad_t* word);	/* 対象の4バイト*/

struct bm_image_s *bm_create(	/* bit-mapイメージを作成する */
	int			width,		/* イメージの幅(bit) */
	int			height,		/* イメージの高さ(bit) */
	bm_pad_t	bm_pad,		/* パディングの単位 */
	bm_order_t	bm_order,	/* バイトオーダーを現す座標系 */
	bm_byte_t	bm_byte,	/* バイト内のビットオーダー */
	bm_onoff_t	bm_onoff,	/* 黒がon('1')かoff('0')か */
	char		init);		/* 初期化イメージ(8-bitパターン)(MSBFirst) */

struct bm_image_s* bm_load(		/* bit_mapファイルをメモリに読み込む */
	char* bm_file,			/* bit-mapファイル名 */
	bm_ftype_t	ftype);		/* bit-mapファイルの形式 */

int				   bm_save(		/* bit_mapファイルに保存する */
	struct	bm_image_s* bm,	/* bit-map */
	char* bm_file);		/* bit_mapファイル名 */

void			   bm_conv(		/* bit-mapイメージを変換する */
	struct bm_image_s* dst,		/* 変換結果 */
	struct bm_image_s* src,		/* 変換もと */
	bm_cnv_t		  mirror);	/* 鏡像変換オプション */

void			   bm_rop(		/* bit-mapイメージにRaster-Operationする */
	struct	bm_image_s* dst,	/* 複写先 */
	struct	bm_image_s* src,	/* 複写元(0/1の場合はall 0/1を仮定する) */
	bm_ope_t	rop);		/* Raster-Operation */

char			  *bm_mkheader(	/* bit-mapファイルのヘッダ情報を作成する */
	char* curr, /*現在のヘッダ*/
	int hsize,	/* ヘッダサイズ(byte) */
	int	isize,	/* bit-mapイメージのサイズ(byte) */
	int	w,		/* bit-mapの幅(bit) */
	int	h);		/* bit-mapの高さ(bit) */

void			   bm_sethead(	/* bit-mapファイルの編集済のヘッダ情報をセットする */
	struct	bm_image_s* bm,	/* bit-map */
	char* header,			/* ヘッダ（bit-mapのファイル形式によって異なる） */
	int		size);			/* ヘッダのサイズ(byte) */

void			   bm_read(		/* bit-mapからビット列を読み出す */
	struct	bm_image_s* dst,/* ビットマップイメージ */
	bm_maxpad_t* pattern);		/* ビット列 */

void			   bm_write(	/* bit-mapにビット列を書き込む */
	struct	bm_image_s* dst,	/* ビットマップイメージ */
	bm_maxpad_t* pattern);			/* ビット列 */

void				   bm_kill(		/* bit-mapイメージを開放する */
	struct	bm_image_s* bm);/* bit-map */

#define			   bm_kill0(bm)	{bm_kill((bm));(bm)=NULL;}	/* 解放後0にする */

/* -----	MASK */
static bm_maxpad_t lsb_mask[]={
	0x00000000,
	0x00000001,0x00000003,0x00000007,0x0000000f,
	0x0000001f,0x0000003f,0x0000007f,0x000000ff,
	0x000001ff,0x000003ff,0x000007ff,0x00000fff,
	0x00001fff,0x00003fff,0x00007fff,0x0000ffff,
	0x0001ffff,0x0003ffff,0x0007ffff,0x000fffff,
	0x001fffff,0x003fffff,0x007fffff,0x00ffffff,
	0x01ffffff,0x03ffffff,0x07ffffff,0x0fffffff,
	0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff
};

static bm_maxpad_t msb_mask[] ={
	0x00000000,
	0x80000000,0xc0000000,0xe0000000,0xf0000000,
	0xf8000000,0xfc000000,0xfe000000,0xff000000,
	0xff800000,0xffc00000,0xffe00000,0xfff00000,
	0xfff80000,0xfffc0000,0xfffe0000,0xffff0000,
	0xffff8000,0xffffc000,0xffffe000,0xfffff000,
	0xfffff800,0xfffffc00,0xfffffe00,0xffffff00,
	0xffffff80,0xffffffc0,0xffffffe0,0xfffffff0,
	0xfffffff8,0xfffffffc,0xfffffffe,0xffffffff
};

