static char sccsid[] =
"@(#)$Workfile: ex_bitope.cpp $$Revision: 6 $$Date: 25/02/04 19:18 $$NoKeywords: $";
/********************************************************************************
 *
 *	bitmap operation : bitmapを操作する exsample
 *		in.bmp(original) →	(original)(X_mirror)
 *							(Y_mirror)(NOT)		外枠・クロス線あり
 ********************************************************************************/

 /* -----	includes */
#include	<iostream>
#include	"bitope.h"
using namespace std;

int
main(
	int argc, 
	char* argv[])
{
	struct bm_image_s* bmin;	/* Windows Paintの第1象限的配置(0:黒)*/
	struct bm_image_s* bmout;	/* Windows Paintの第1象限的配置(0:黒)*/
	struct bm_image_s* bmin4th;	/* 以下のbmpはbitope処理前提の第4象限的配置(1:黒) */
	struct bm_image_s* bmout4th;/* (original)(X_mirror)(Y_mirror)(NOT)と外枠・クロス線を含む */
	struct bm_image_s* bmtmp4th;/* 鏡像反転用　*/

	/* -----	procedure */

	if (argc != 3) {
		cout << "Usage:ex_bitope in.bmp out.bmp" << endl;
		exit(1);
	}

	/* bit_mapを読み込む */
	bmin = bm_load(argv[1], BM_PAINT);
	if (bmin == NULL) {
		cout << "error:bm_load in.bmp:" << argv[1] << endl;
		exit(1);
	}

	/* bit_map create */
	bmin4th = bm_create(bmin->w,bmin->h, BM_PAD32, BM_4TH, BM_MSBFirst, BM_ON, 0x00);

	bmout4th = bm_create(1 + (bmin->w +1+ bmin->w) + 1, 1 + (bmin->h +1+ bmin->h) + 1, BM_PAD32, BM_4TH, BM_MSBFirst, BM_ON, 0x00);
	bmout4th->hsize = bmin->hsize;
	bmout4th->header = bm_mkheader(NULL, bmout4th->hsize, bmout4th->isize, bmout4th->w, bmout4th->h);

	bmtmp4th = bm_create(bmin->w, bmin->h, BM_PAD32, BM_4TH, BM_MSBFirst, BM_ON, 0x00);	/* X,Y方向鏡像反転用　*/
	bmtmp4th->hsize = bmin->hsize;
	bmtmp4th->header = bm_mkheader(NULL, bmtmp4th->hsize, bmtmp4th->isize, bmtmp4th->w, bmtmp4th->h);

	bmout = bm_create(bmout4th->w, bmout4th->h, BM_PAD32, BM_1ST, BM_MSBFirst, BM_OFF, '\xFF');
	bmout->hsize = bmin->hsize;
	bmout->header = bm_mkheader(NULL, bmout->hsize, bmout->isize, bmout->w, bmout->h);

	/* 入力のビットマップ(in.bmp)を第4象限的に配置する */
	bm_conv(bmin4th, bmin, BM_NULL);
#ifdef _DEBUG
	bm_save(bmin4th, (char*)"bmin4th_image.bin");
	bmin4th->hsize = bmin->hsize;
	bmin4th->header = bm_mkheader(NULL, bmin4th->hsize, bmin4th->isize, bmin4th->w, bmin4th->h);
	bm_save(bmin4th, (char*)"bmin4th.bmp");
#endif

	/* 外枠線を引く*/
	bmout4th->x = 0;
	bmout4th->y = 0;
	bmout4th->w = 1 + (bmin->w +1+ bmin->w) + 1;
	bmout4th->h = 1;
	bm_rop(bmout4th, BM_SRC1, BM_set);/* 上 */
	bmout4th->x = 0;
	bmout4th->y = 0;
	bmout4th->w = 1;
	bmout4th->h = 1 + (bmin->h + 1 + bmin->h) + 1;
	bm_rop(bmout4th, BM_SRC1, BM_set);/* 左 */
	bmout4th->x = 1 + (bmin->w + 1 + bmin->w) + 1 -1;
	bmout4th->y = 0;
	bmout4th->w = 1;
	bmout4th->h = 1 + (bmin->h + 1 + bmin->h) + 1;
	bm_rop(bmout4th, BM_SRC1, BM_set);/* 右 */ 
	bmout4th->x = 0;
	bmout4th->y = 1 + (bmin->h + 1 + bmin->h) + 1 - 1;
	bmout4th->w = 1 + (bmin->w + 1 + bmin->w) + 1;
	bmout4th->h = 1;
	bm_rop(bmout4th, BM_SRC1, BM_set);/* 下 */
#ifdef _DEBUG
	bm_save(bmout4th, (char*)"tmp00.bmp");
#endif

	/* 出力用に配置する */
	bmin4th->x = 0;
	bmin4th->y = 0;
	bmout4th->x = 1;
	bmout4th->y = 1;
	bmout4th->w = bmin->w;
	bmout4th->h = bmin->h;
	bm_rop(bmout4th, bmin4th, BM_set);	/* オリジナル */
#ifdef _DEBUG
	bm_save(bmout4th, (char*)"tmp01.bmp");
#endif

	bmin4th->x = 0;
	bmin4th->y = 0;
	bmout4th->x = 1+ bmin->w + 1;
	bmout4th->y = 1+ bmin->h + 1;
	bmout4th->w = bmin->w;
	bmout4th->h = bmin->h;
	bm_rop(bmout4th, bmin4th, BM_orN);	/* 白黒反転 */
#ifdef _DEBUG
	bm_save(bmout4th, (char*)"tmp02.bmp");
#endif


	/* X方向鏡像反転を得る */
	bm_conv(bmtmp4th, bmin4th, BM_X_MIRROR);/* X方向鏡像反転を得る(パディングを含むiwまで使用する) */
	bmtmp4th->header = bm_mkheader(bmtmp4th->header, bmtmp4th->hsize, bmtmp4th->isize, bmtmp4th->iw, bmtmp4th->h);
#ifdef _DEBUG
	bm_save(bmtmp4th, (char*)"tmp03.bmp");
#endif

	bmtmp4th->x = bmtmp4th->iw- bmtmp4th->w;/* パディング部分をオミット */
	bmtmp4th->y = 0;
	bmout4th->x = 1+ bmin->w + 1;
	bmout4th->y = 1;
	bmout4th->w = bmin->w;
	bmout4th->h = bmin->h;
	bm_rop(bmout4th, bmtmp4th, BM_set);/* X方向鏡像反転 */
#ifdef _DEBUG
	bm_save(bmout4th, (char*)"tmp04.bmp");
#endif


	/* Y方向鏡像反転を得る */
	bm_conv(bmtmp4th, bmin4th, BM_Y_MIRROR);
	bmtmp4th->header = bm_mkheader(bmtmp4th->header, bmtmp4th->hsize, bmtmp4th->isize, bmtmp4th->w, bmtmp4th->h);
#ifdef _DEBUG
	bm_save(bmtmp4th, (char*)"tmp05.bmp");
#endif

	bmtmp4th->x = 0;
	bmtmp4th->y = 0;
	bmout4th->x = 1;
	bmout4th->y = 1 + bmin->h + 1;
	bmout4th->w = bmin->w;
	bmout4th->h = bmin->h;
	bm_rop(bmout4th, bmtmp4th, BM_set);/* Y方向鏡像反転 */
#ifdef _DEBUG
	bm_save(bmout4th, (char*)"tmp06.bmp");
#endif

	/* クロス線を引く*/
	bmout4th->x = 1 + bmin->w;
	bmout4th->y = 0;
	bmout4th->w = 1;
	bmout4th->h = 1 + (bmin->h + 1 + bmin->h) + 1;
	bm_rop(bmout4th, BM_SRC1, BM_set);/* 縦 */
	bmout4th->x = 0;
	bmout4th->y = 1 + bmin->h;
	bmout4th->w = 1 + (bmin->w + 1 + bmin->w) + 1;
	bmout4th->h = 1;
	bm_rop(bmout4th, BM_SRC1, BM_set);/* 横 */
#ifdef _DEBUG
	bm_save(bmout4th, (char*)"tmp07.bmp");
#endif

	/* 4つのbmpを配置したビットマップ(out.bmp)を出力する */
	bm_conv(bmout, bmout4th, BM_NULL);
	bm_save(bmout, (char*)argv[2]);

	/* 解放する */
	bm_kill(bmin);
	bm_kill(bmin4th);
	bm_kill(bmtmp4th);
	bm_kill(bmout4th);
	bm_kill(bmout);
}


