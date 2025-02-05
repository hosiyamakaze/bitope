static char sccsid[] =
"@(#)$Workfile: ex_base64.cpp $$Revision: 7 $$Date: 25/02/04 21:34 $$NoKeywords: $";
/********************************************************************************
 *
 *	bitmap operation : bit列を操作する exsample
 *      source text →	Base64encode/decode
 *      Base64 text →  Base64decode/encode
 * 
 ********************************************************************************/

 /* -----	includes */
#include	<iostream>
#include    <string>
#include	"bitope.h"
using namespace std;


/* -----    static variables */
static string strB64TBL = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

string
base64_encode(
    const string& strSrc)
{
    string  out;
    struct bm_image_s* bmsrc;	/* 入力をビット列に変換する*/
    struct bm_image_s* bmsrc2;	/* 入力をビット列に変換する*/
    bm_maxpad_t	buf;		    /* イメージバッファ */
    int     bitlen;  	        /* 入力のビット数 */
    int	    b64len;             /* Base64code(6bit単位)の桁数 */
    char    chr;				/* endecodeする文字のコード値 */

    /* -----    procedure */

    bitlen = strSrc.size() * 8;
    b64len = bitlen / 6 + (bitlen % 6 == 0 ? 0 : 1);

    //source textを8bit毎にBMPに読み込む
    bmsrc = bm_create(bitlen, 1, BM_PAD32, BM_4TH, BM_MSBFirst, BM_ON, 0x00);
    memcpy(bmsrc->image, strSrc.c_str(), bmsrc->isize);
#ifdef _DEBUG
    {
        struct bm_image_s* bmout = bm_create(bmsrc->w, bmsrc->h, BM_PAD32, BM_1ST, BM_MSBFirst, BM_OFF, '\xFF');
        bm_conv(bmout, bmsrc, BM_NULL);
        bmout->hsize = 62;
        bmout->header = bm_mkheader(NULL, bmout->hsize, bmout->isize, bmout->w, bmout->h);
        bm_save(bmout, (char*)"bmsrcStr_source.bmp");
        bm_kill(bmout);

        // x幅を9文字(*8bit)のBMPにする
        bmsrc2 = bm_create(9 * 8, bitlen / (9 * 8) + (bitlen % (9 * 8) == 0 ? 0 : 1), BM_PAD32, BM_4TH, BM_MSBFirst, BM_ON, 0x00);
        bmout = bm_create(bmsrc2->w, bmsrc2->h, BM_PAD32, BM_1ST, BM_MSBFirst, BM_OFF, '\xFF');
        bmsrc2->x = 0;bmsrc2->y = 0;bmsrc2->w = 8;bmsrc2->h = 1;
        for (int i = 1;i <= strSrc.size();i++) {
            chr = strSrc.at(i - 1);
            buf = 0;
            memcpy(&buf, &chr, 1);
            bm_swab(&buf);
            bm_write(bmsrc2, &buf);
            if (i % 9 == 0) {
                bmsrc2->x = 0;
                bmsrc2->y++;
            }
            else {
                bmsrc2->x += 8;
            }
        }
        bm_conv(bmout, bmsrc2, BM_NULL);
        bmout->hsize = 62;
        bmout->header = bm_mkheader(NULL, bmout->hsize, bmout->isize, bmout->w, bmout->h);
        bm_save(bmout, (char*)"bmsrcStr_by9matrix.bmp");
        bm_kill(bmout);
		bm_kill(bmsrc2);
    }
#endif
    /*　source textを6bit毎に読み込む */
	bmsrc->x = 0; bmsrc->y = 0; bmsrc->w = 6; bmsrc->h = 1;

    // x幅を12文字(*00+6bit)のBMPにする
    bmsrc2 = bm_create(12 * 8, bitlen / (9 * 8) + (bitlen % (9 * 8) == 0 ? 0 : 1), BM_PAD32, BM_4TH, BM_MSBFirst, BM_ON, 0x00);
    /* 6bitを8bit(下6bit)に移し替える*/
    bmsrc2->x = 2; bmsrc2->y = 0; bmsrc2->w = 6; bmsrc2->h = 1;
    for (int i=1;bmsrc->x < bitlen;bmsrc->x += bmsrc->w, i++) {
        bm_read(bmsrc, &buf);
        bm_write(bmsrc2, &buf);
        if (i %  12 == 0) {
            bmsrc2->x=2;
            bmsrc2->y++;
		}
		else {
			bmsrc2->x += 8;
		}
    }
#ifdef _DEBUG
  {
      struct bm_image_s* bmout = bm_create(12 * 8, bitlen / (9 * 8) + (bitlen % (9 * 8) == 0 ? 0 : 1), BM_PAD32, BM_1ST, BM_MSBFirst, BM_OFF, '\xFF');
      bm_conv(bmout, bmsrc2, BM_NULL);
      bmout->hsize = 62;
      bmout->header = bm_mkheader(NULL, bmout->hsize, bmout->isize, bmout->w, bmout->h);
      bm_save(bmout, (char*)"bmsrcStr_Base64_by12matrix.bmp");
      bm_kill(bmout);
    }
#endif
    bmsrc2->x = 0;  /* Base64codeを読み取る*/
    bmsrc2->y = 0;
    bmsrc2->w = 8;
    bmsrc2->h = 1;
    for (int i = 1; i <= b64len;i++) {
        bm_read(bmsrc2,&buf);
        out.push_back(strB64TBL.at(buf>>(32-8)));
        if (i % 12 == 0) {
            bmsrc2->x = 0;
            bmsrc2->y++;
		}
		else {
            bmsrc2->x += bmsrc2->w;
		}
    }
    while (out.size() % 4) out.push_back('=');

    return out;
}

string
base64_decode(
    const string& strB64)
{
    struct bm_image_s* bmsrc;	/* 入力をビット列に変換する*/
    bm_maxpad_t	buf;		    /* イメージバッファ */
    int     bitlen;  	        /* decodeに必要なビット数 */
	char    chr;				/* decodeする文字のコード値 */

    /* -----    procedure */

    bitlen = strB64.size() * 6;
    bmsrc = bm_create(bitlen + 8/*for terminate*/, 1, BM_PAD32, BM_4TH, BM_MSBFirst, BM_ON, 0x00);
    bmsrc->x = 0;
    bmsrc->y = 0;
    bmsrc->w = 6;
    bmsrc->h = 1;
    for (int i = 0;i < strB64.size();i++) {
        chr = strB64.at(i);
        if ((chr = strB64TBL.find(chr)) != string::npos) {
            chr <<= 2;  /* 6bitを左寄せにする*/
            buf = 0;
            memcpy(&buf, &chr, 1);
            bm_swab(&buf);/* swap byteを変換する */
            bm_write(bmsrc, &buf);
            bmsrc->x += bmsrc->w;
        }
    }
#ifdef _DEBUG
    struct bm_image_s* bmout = bm_create(bitlen + 8/*for terminate*/, 1, BM_PAD32, BM_1ST, BM_MSBFirst, BM_OFF, '\xFF');
    bm_conv(bmout, bmsrc, BM_NULL);
    bmout->hsize = 62;
    bmout->header = bm_mkheader(NULL, bmout->hsize, bmout->isize, bmout->w, bmout->h);
    bm_save(bmout, (char*)"bmB64Str_decorded.bmp");
    bm_kill(bmout);
#endif
   
    return string(bmsrc->image);
}

int
main(
	int argc,
	char* argv[])
{
	/* -----	procedure */

    string strIn;
    string strb64;

    while (true) {
        cout << "Enter source text (for Enter to Base64): ";

        // 入力が空の場合、ループを終了
        if (!getline(cin, strIn) || strIn.empty()) {
            cout << "Enter Base64 text (for Enter to exit): ";
            // 入力が空の場合、ループを終了
            if (!getline(cin, strb64) || strb64.empty()) {
                break;
            }
            strIn = base64_decode(strb64);
            // 入力されたテキストを表示
            cout << "Base64 Decode: " << strIn << endl;
            cout << "Base64 Encode: " << base64_encode(strIn) << endl;
        
        }
        else {
            strb64 = base64_encode(strIn);
            // 入力されたテキストを表示
            cout << "Base64 Encode: " << strb64 << endl;
            cout << "Base64 Decode: " << base64_decode(strb64) << endl;
        }

    }

    cout << "Done." << endl;
    return 0;
}
