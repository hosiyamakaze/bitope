static char sccsid[]=
"@(#)$Workfile: bitope.cpp $$Revision: 12 $$Date: 25/02/04 19:13 $$NoKeywords: $";
/*
*****************************************************************************
*
*    bitope.c:	�a�h�s�|�l�`�o�I�y���[�V����
*
*
*    ���L�����F
*        �Ebit�P�ʂ̓]���������T�|�[�g���܂��B
*        �E���m�N���p�^�[��(DEPTH=1)�̂ݑΉ����܂��B
*		 �E��������ł͍��ɑΉ�����r�b�g��on('1')�Ƃ��܂�
*        �E���̊T�O���T�|�[�g���܂��B�i�ꕔ�̎d�l�͈�ʓI�ł͂Ȃ��j
*			bit_map_pad:	�r�b�g�}�b�v�̃p�f�B���O�i�������[�h���j
*			bit_map_order:	�o�C�g�̕��я��i���W�n�Ɍ����Ă܂��j
*			bit_byte_order:	�o�C�g���̃r�b�g�̕��я�
*
*****************************************************************************
*/

#include	<stdio.h>
#include	<memory>
#include	<cstring>
#include	"bitope.h"

/* -----	define */
#define abs(x)	((x)<0?(-(x)):(x))		/* ��Βl */

/* -----	static function */

/*
*********************************************************************
* bm_msblsb:	�p�f�B���O�P�ʂ�bit�C���[�W���������]������
*********************************************************************
*/
static void
bm_msblsb(
	bm_maxpad_t	src,	/* ����bit�C���[�W */
	bm_maxpad_t*	dst,	/* �������]���bit�C���[�W */
	int			bits)	/* src�̗L����ʂ������� */
{
	unsigned int	srcpos, dstpos;	/* �r�b�g�ʒu */
	int				i;

	/* -----    procedure */

	*dst = 0x0;					/* NULL */
	srcpos = 0x80000000;		/* src��MSB����bit�����o�� */
	dstpos = 0x00000001;		/* dst��LSB����bit���Z�b�g���� */

	for (i = 0;i < bits;i++, srcpos >>= 1, dstpos <<= 1) {
		if (src & srcpos) {
			(*dst) |= dstpos;
		}
	}
}

/*
**********************************************************************
*	bm_getpos:	�ʒu���߂����čŏ��̃p�f�B���O�P�ʃT�C�Y�������o��
**********************************************************************
*/
static void					/* 0:�f�[�^����/-1:�f�[�^�I�� */
bm_getpos(
	struct bm_image_s* dst,	/* bit-map�C���[�W */
	int				  x,	/* x���W(bit�P��) */
	int				  y,	/* y���W(bit�P��) */
	bm_maxpad_t* pattern,		/* bit�p�^�[��(��ʃo�C�g�ɃZ�b�g����) */
	int				   len)	/* �ǂݍ��݃r�b�g��(�p�f�B���O�T�C�Y�ȓ�) */
{
	int			pos;		/* �o�C�g�ʒu */
	int			offset;		/* �r�b�g�I�t�Z�b�g(bit��) */
	int			eol = 0;	/* -1:end of line/0:else */
	bm_maxpad_t	nextptn;	/* ���̃p�f�B���O�̃r�b�g�p�^�[�� */

	/* -----	procedure */

		/* �擪�r�b�g�̑��݂���p�f�B���O�ʒu(�o�C�g�P��)���Z�o���� */
	pos = x / dst->bits * dst->bytes;	/* x�������Z�o */
	pos += (dst->iw_byte * y);		/* y������ǉ� */
	offset = x % dst->bits;	/* �p�f�B���O���̃I�t�Z�b�g */

	/* �p�f�B���O�P�ʃT�C�Y���ȓ������o�� */
	*pattern = 0x0;
	memcpy((char*)pattern, dst->image + pos, dst->bytes);
	bm_swab(pattern);	/* swap byte��ϊ����� */
	if (x + len > dst->x + dst->w) {
		eol = -1;	/* ����ł��̍s�͓��͏I�� */
	}

	/* �p�f�B���O���I�t�Z�b�g����␳���� */
	if (offset != 0) {
		/* �C���[�W��Ɏ��̃p�f�B���O������Γǂݍ��� */
		nextptn = 0x0;
		if ((offset + len > dst->bits) && eol != -1) {
			/* ���̂P�p�f�B���O����s���r�b�g��₤ */
			memcpy((char*)&nextptn, dst->image + pos + dst->bytes, dst->bytes);
			bm_swab(&nextptn);	/* swap byte��ϊ����� */
			(*pattern) = (((*pattern) << offset)
				& ~lsb_mask[offset])	/* ��ʃp�f�B���O�̏�� */
				|
				((nextptn >> (dst->bits - offset))
					& lsb_mask[offset]);	/* ���ʃp�f�B���O�̉��� */
		}
		else {
			(*pattern) <<= offset;
		}
	}
	(*pattern) &= msb_mask[len];
}

/*
**********************************************************************
*	bm_setpos:	�ʒu���߂����čŏ��̃p�f�B���O�P�ʃT�C�Y������������
**********************************************************************
*/
static void
bm_setpos(
	struct bm_image_s* dst,		/* bit-map �C���[�W */
	int				  x,		/* x���W(bit�P��) */
	int				  y,		/* y���W(bit�P��) */
	bm_maxpad_t		  pattern,	/* bit�p�^�[��(��ʃo�C�g�ɃZ�b�g����) */
	int				  len)		/* ��������bit��(�p�f�B���O�T�C�Y�ȓ�) */
{
	bm_maxpad_t	currptn, nextptn;	/* �������݈ʒu��bit�p�^�[�� */
	bm_maxpad_t	mask;				/* pattern�̗L��bit-mask */
	int			nextlen;			/* ���̃p�f�B���O����̓ǂݍ���bit�� */
	int			offset;				/* �p�f�B���O���I�t�Z�b�g(bit) */
	int			pos;				/* �ǂݍ��݃p�f�B���O�ʒu(byte) */

	/* -----	procedure */

		/* pattern�̗L���r�b�g������mask������ */
	if (x + len > dst->x + dst->w) len = dst->x + dst->w - x;	/* len��␳���� */
	mask = msb_mask[len];

	/* �擪�r�b�g�̑��݂���p�f�B���O�ʒu(�o�C�g�P��)���Z�o���� */
	pos = x / dst->bits * dst->bytes;	/* x�������Z�o */
	pos += (dst->iw_byte * y);		/* y������ǉ� */
	offset = x % dst->bits;		/* �p�f�B���O��offset(bit) */

	if (offset == 0 && len == dst->bits) {
		currptn = 0;
	}
	else {
		/* �������݈ʒu�̌��݂̒l�����o�� */
		bm_getpos(dst, x / dst->bits * dst->bits, y, &currptn, dst->bits);
	}
	if (offset + len > dst->bits) {
		/* ���̏������݈ʒu�̌��݂̒l�����o�� */
		nextlen = dst->bits * 2 - offset - len;
		bm_getpos(dst, (x + len) / dst->bits * dst->bits, y, &nextptn, dst->bits);
	}

	/* �������݈ʒu�̃p�f�B���O�̃C���[�W��ҏW���������� */
	if (offset == 0) {
		currptn = (pattern & mask) | (currptn & ~mask);
		bm_swab(&currptn);	/* swap byte��ϊ����� */
		memcpy(dst->image + pos, (char*)&currptn, dst->bytes);
	}
	else {
		currptn = (currptn & ~(mask >> offset))
			|
			((pattern & mask) >> offset);
		bm_swab(&currptn);	/* swap byte��ϊ����� */
		memcpy(dst->image + pos, (char*)&currptn, dst->bytes);
		if (offset + len > dst->bits) {
			/* ���̏������݈ʒu�̃C���[�W��ҏW���������� */
			nextptn = (nextptn & lsb_mask[nextlen])
						|
					  ((pattern & mask) << (dst->bits - offset));
			bm_swab(&nextptn);	/* swap byte��ϊ����� */
			memcpy(dst->image + pos + dst->bytes, (char*)&nextptn, dst->bytes);
		}
	}
}


/* -----	global function */

/*
*********************************************************************
* bm_swab:	swap byte�����s����
*********************************************************************
*/
void
bm_swab(		/* swab */
	bm_maxpad_t* word)	/* �Ώۂ�4�o�C�g*/
{
	char	src[4];	/* �ϊ��p��4�o�C�g*/

	/* ----    procdure */

	memcpy(src, word, 4);	/* swab */
	*(((char*)word) + 3) = *(src + 0);
	*(((char*)word) + 2) = *(src + 1);
	*(((char*)word) + 1) = *(src + 2);
	*(((char*)word) + 0) = *(src + 3);
}

/*
*********************************************************************
* bm_create:	bit-map�C���[�W���쐬����
*********************************************************************
*/
struct bm_image_s *
bm_create(
int			width,		/* �C���[�W�̕�(bit) */
int			height,		/* �C���[�W�̍���(bit) */
bm_pad_t	bm_pad,		/* �p�f�B���O�̒P�� */
bm_order_t	bm_order,	/* �o�C�g�I�[�_�[���������W�n */
bm_byte_t	bm_byte,	/* �o�C�g���̃r�b�g�I�[�_�[ */
bm_onoff_t	bm_onoff,	/* ����on('1')��off('0')�� */
char		init)		/* �������C���[�W(8-bit�p�^�[��)(MSBFirst) */
{
	struct bm_image_s *bm;	/* �쐬���ꂽbit-map */

/* -----    procedure */

	/* bit-map�̈���m�ۂ��A��`���� */
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
	bm->bytes = BM_PADBYTES(bm->bm_pad);	/* �p�f�B���O��byte�� */
	bm->bits = BM_PADBITS(bm->bm_pad);		/* �p�f�B���O��bit�� */
	/* iw�ƃp�f�B���O�T�C�Y�ōs�̃o�C�g�����Z�o���� */
	bm->iw = (bm->w/ BM_PADBITS(bm->bm_pad) + (bm->w% BM_PADBITS(bm->bm_pad) ==0?0:1))* BM_PADBITS(bm->bm_pad);
	bm->iw_byte = bm->iw/ BM_PADBITS(bm->bm_pad) * BM_PADBYTES(bm->bm_pad);

	/* ���K�����������������T�C�Y���Z�o���A���������m�ۂ��� */
	bm->isize = bm->iw_byte * height;
	if((bm->image = (char *)malloc(bm->isize))==NULL){
		free(bm);
		bm = NULL;
		goto exit_procedure;
	}

	/* bit-map���������C���[�W�ɂ��� */
	memset(bm->image,init, bm->isize);

exit_procedure:
	return(bm);
}

/*
*********************************************************************
* bm_load:	bit-map�t�@�C�����������ɓǂݍ���
*********************************************************************
*/
struct bm_image_s *
bm_load(
char		*bm_file,	/* bit-map�t�@�C���� */
bm_ftype_t	ftype)		/* bit-map�t�@�C���̌`�� */
{
	FILE 	*bitmapfp;		/* bit-map�t�@�C���ւ̃|�C���^ */
	char	header[256];	/* �w�b�_�o�b�t�@ */
	struct bm_image_s *bm;	/* bit-map */
	int		width,height;	/* bit-map�̕��ƍ���(bit) */

/* -----	procedure */

	bitmapfp = NULL;
	bm = NULL;

	if(ftype!=BM_PAINT/* �y�C���g�u���V */){
		goto exit_procedure;
	}

	/* bit-map�t�@�C�����I�[�v������ */
	if(fopen_s(&bitmapfp, bm_file, "rb") != 0){
		goto exit_procedure;
	}

	/* -----	for �y�C���g�u���V(MS-Windows) */
	/* �w�b�_���(62�o�C�g)����͂��� */
	memset(header,0x00,sizeof(header));
	if(fread(header, 62, 1, bitmapfp) != 1 ) {
		goto exit_procedure;
	}
	if(strncmp(header, "BM", 2) != NULL) {
		goto exit_procedure;
	}


	/* bit-map�̃T�C�Y�����o�����������m�ۂ��� */
	memcpy((char *)&width,header+0x12,4);	/* swab */
	memcpy((char *)&height,header+0x16,4);	/* swab */
	if((bm = bm_create(width,height,BM_PAD32,BM_1ST,BM_MSBFirst,BM_OFF,0x00))==NULL){
		goto exit_procedure;
	}

	/* �������C���[�W�Ƀw�b�_���Z�b�g���� */
	bm->hsize  = 62;
	if((bm->header = (char *)malloc(bm->hsize))==NULL){
		free(bm);
		bm = NULL;
		goto exit_procedure;
	}
	memcpy(bm->header,header,bm->hsize);

	/* bit�C���[�W����͂���(�p�f�B���O�P�ʂ͈�v���Ă���) */
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
* bm_mkheader:	bit-map�w�b�_���쐬����
*********************************************************************
*/
char*
bm_mkheader(
	char* curr, /*���݂̃w�b�_(NULL:�������m�ۂ���)*/
	int hsize,	/* �w�b�_�T�C�Y(byte)(62 for .bmp) */
	int	isize,	/* bit-map�C���[�W�̃T�C�Y(byte) */
	int	w,		/* bit-map�̕�(bit) */
	int	h)		/* bit-map�̍���(bit) */
{
	char* header;	/* bit-map�w�b�_ */
	int	  fsize;	/* �t�@�C���T�C�Y */

	/* -----	procedure */

	fsize = isize + hsize;
	if (curr == NULL) {
		header = (char*)malloc(hsize < 62 ? 62 : hsize);
		memset(header, 0x00, hsize);
	}
	else {
		header = curr;
	}

	memcpy(header, (char *)"BM", 2);				/* �t�@�C���^�C�v */
	memcpy(header + 0x02, (char*)&fsize, 4);		/* �t�@�C���T�C�Y */
	memcpy(header + 0x0A, (char*)"\x3E\x00\x00\x00", 4);	/* �I�t�Z�b�g */
	memcpy(header + 0x0E, (char*)"\x28\x00\x00\x00", 4);	/* �w�b�_�T�C�Y */
	memcpy(header + 0x12, (char*)&w, 4);			/* �� */
	memcpy(header + 0x16, (char*)&h, 4);			/* ���� */
	memcpy(header + 0x1A, (char*)"\x01\x00", 2);	/* �v���[���� */
	memcpy(header + 0x1C, (char*)"\x01\x00", 2);	/* �r�b�g�� */
	memcpy(header + 0x26, (char*)"\x11\x0B\x00\x00", 4);	/* ���������̉𑜓x */
	memcpy(header + 0x2A, (char*)"\x11\x0B\x00\x00", 4);	/* ���������̉𑜓x */
	memcpy(header + 0x2E, (char*)"\x02\x00\x00\x00", 4);	/* �v���[���� */
	memcpy(header + 0x32, (char*)"\x00\x00\x00\x00", 4);	/* �r�b�g�� */
	memcpy(header + 0x36, (char*)"\x00\x00\x00\x00", 4);	/* �s��(�J���[�}�X�N) */
	memcpy(header + 0x3A, (char*)"\xFF\xFF\xFF\x00", 4);	/* �s��(�J���[�}�X�N) */

	return(header);	
}

/*
*********************************************************************
* bm_save:	bit-map�t�@�C���ɕۑ�����
*********************************************************************
*/
int
bm_save(
struct bm_image_s *bm,		/* bit-map */
char			  *bm_file)	/* bit_map�t�@�C���� */
{
	FILE 	*bitmapfp;		/* bit-map�t�@�C���ւ̃|�C���^ */
	int		 stat;

/* -----	procedure */

	bitmapfp = NULL;
	stat 	 = -1;

	/* bit-map�t�@�C�����I�[�v������ */
	if(fopen_s(&bitmapfp, bm_file, "wb") != 0){
		goto exit_procedure;
	}

	/* �w�b�_�����o�͂��� */
	if(bm->header!=NULL && bm->hsize>0){
		if(fwrite(bm->header,bm->hsize,1,bitmapfp)==0){
			goto exit_procedure;
		}
	}

	/* bit�C���[�W���o�͂��� */
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
* bm_sethead:	bit-map�t�@�C���̕ҏW�ς̃w�b�_�����Z�b�g����
**********************************************************************
*/
void
bm_sethead(
struct bm_image_s *bm,	/* bit-map */
char	*header,		/* �w�b�_�ibit-map�̃t�@�C���`���ɂ���ĈقȂ�j */
int		 size)			/* �w�b�_�̃T�C�Y(byte) */
{
/* -----	procedure */

	/* �w�b�_�̃����������蓖�Ă� */
	if(bm->hsize<size){
		if(bm->header!=0) free(bm->header);
		bm->header = (char *)malloc(size);
		bm->hsize  = size;
	}else{
		bm->hsize = size;
	}
	memset(bm->header,0x00,bm->hsize);
	
	/* �w�b�_�̓��e���Z�b�g���� */
	memcpy(bm->header,header,bm->hsize);
}

/*
*********************************************************************
* bm_rop:	�C���[�W���I�y���[�V�����ɏ]���ď�������
*			��{�C���[�W(�I�[�_�[:BM_4TH,��:BM_ON)��O��Ƃ���
*			�p�f�B���O�T�C�Y�C�r�b�g�I�[�_�[�͔C�ӂł悢
*********************************************************************
*/
void
bm_rop(
struct bm_image_s *dst,		/* ���ʐ� */
struct bm_image_s *src,		/* ���ʌ�(0/1�̏ꍇ��all 0/1�����肷��) */
bm_ope_t		   rop)		/* Raster-Operation */
{
	bm_maxpad_t	*dst_bits;	/* dst�̃r�b�g�� */
	bm_maxpad_t	*src_bits;	/* src�̃r�b�g�� */
	int			size;		/* �������T�C�Y(byte) */
	int			nofpad;		/* �p�f�B���O�� */
	int			save_y_src,save_y_dst,save_h_dst;/* �l��ۑ����� */
	int			h,i;

/* -----	procedure */

	/* �P���C��(h=1)���̃r�b�g��̃��������m�ۂ��� */
	nofpad	= dst->w/BM_PADBITS_MAX+(dst->w%BM_PADBITS_MAX==0?0:1);
	size	= nofpad * BM_PADBYTES_MAX;
	dst_bits = (bm_maxpad_t *)malloc(size);
	src_bits = (bm_maxpad_t *)malloc(size);

	/* �l��ۑ����Ă��� */
	save_y_dst = dst->y;
	if(src!=BM_SRC0 && src!=BM_SRC1) save_y_src = src->y;
	save_h_dst = dst->h;

	/* ���C������(dst->h)���s���� */
	for(dst->h=1,h=0;h<save_h_dst;h++){

		/* src�r�b�g���ǂݍ��� */
		memset(src_bits,0x00,size);
		if(src==BM_SRC0){
			/* all '0'�����肷�� */
		}else if(src==BM_SRC1){
			/* all '1'�����肷�� */
			memset(src_bits,0xff,size);
		}else{
			/* �r�b�g���ǂݍ��� */
			src->w = dst->w;
			src->h = dst->h;
			bm_read(src,src_bits);
		}
	
		/* dst�r�b�g���ǂݍ��� */
		memset(dst_bits,0x00,size);
		bm_read(dst,dst_bits);
	
		/* �I�y���[�V���������s���� */
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

		/* dst�Ƀr�b�g����������� */
		bm_write(dst,dst_bits);

		dst->y ++;
		if(src!=BM_SRC0 && src!=BM_SRC1) src->y++;
	}

	/* �l�𕜌����� */
	dst->y = save_y_dst;
	dst->h = save_h_dst;
	if(src!=BM_SRC0 && src!=BM_SRC1){
		src->y = save_y_src;
		src->h = dst->h;
	}

	/* ��������������� */
	free(dst_bits);
	free(src_bits);
}

/*
**********************************************************************
*	bm_conv:	�I�[�_�[(bm_order,bm_byte)�C����(bm_onoff)��ϊ�����
*				�C���[�W�T�C�Y�C�p�f�B���O�T�C�Y�͈�v���Ă��邱��
* 				�C���[�W�T�C�Y�̓p�f�B���O�̐����{�ł��邱�� 
**********************************************************************
*/
void
bm_conv(
struct bm_image_s *dst,		/* �ϊ����� */
struct bm_image_s *src,		/* �ϊ����� */
bm_cnv_t		  mirror)	/* �����ϊ��I�v�V���� */
{
	int		x_hi,y_hi;		/* �ǂݍ��ݕ��� */
	int		getpos;			/* �ǂݍ��݈ʒu */
	int		x_len;			/* x�̑���(src���̃p�f�B���O�P�ʂƂ���(byte)) */
	int		y_len;			/* y�̑���(�P���C���̒���(byte)) */
	int		y_lenpos;
	int		setpos,setx_len,sety_len;	/* �������ݑ��̈ʒu�Ȃ� */
	int		h,w,w_pad;		/* ���C��,�p�f�B���O�J�E���^,�P���C���̃p�f�B���O */
	bm_maxpad_t	buf;		/* �C���[�W�o�b�t�@ */

/* -----	procedure */

	/* �p�f�B���O�I�[�_�[�̓ǂݍ��ݕ����C�������Z�o���� */
	x_hi = ((0xf0&(int)dst->bm_order)==(0xf0&(int)src->bm_order)?0x0:0x1);
	x_hi = ((mirror&BM_X_MIRROR)?(~x_hi&0x1):x_hi);/* �������]�w��Ȃ甽�΂ɂ��� */
	x_len = BM_PADBYTES(src->bm_pad);
	x_len = (x_hi?(-x_len):x_len);
	setx_len = abs(x_len);

	/* ���C���I�[�_�[�̓ǂݍ��ݕ����C�������Z�o���� */
	y_hi = ((0x0f&(int)dst->bm_order)==(0x0f&(int)src->bm_order)?0x0:0x1);
	y_hi = (mirror&BM_Y_MIRROR?(~y_hi&0x1):y_hi);	/* �������]�w��Ȃ甽�΂ɂ��� */
	y_len = src->iw/BM_PADBITS(src->bm_pad)*BM_PADBYTES(src->bm_pad);
	w_pad = src->iw/BM_PADBITS(src->bm_pad);
	y_len = (y_hi?(-y_len):y_len);
	y_lenpos = (x_hi==y_hi?0:(y_len*2));
	sety_len = 0;

	/* �ǂݍ��ݐ擪�ʒu���Z�o���� */
	getpos = (x_hi?abs(y_len)+x_len:0);			/* x�I�t�Z�b�g�����߂� */
	getpos += (y_hi?abs(y_len)*(src->ih-1):0);	/* y�����ɃV�t�g���� */
	setpos = 0;

	/* �]������ */
	//for (int i = 0;i < src->isize;i++) { dst->image[i] = src->image[i]; }

	for(h=0;h<src->ih;h++,getpos+=y_lenpos,setpos+=sety_len){
		for(w=0;w<w_pad;w++,getpos+=x_len,setpos+=setx_len){
			memcpy((char*)&buf,src->image+getpos,setx_len);	/* �ǂ݂��� */
			bm_swab(&buf);	/* swap byte��ϊ����� */
			if(mirror&BM_X_MIRROR && dst->bm_byte==src->bm_byte){
				/* �������ɋ������]�̏ꍇ�́A�r�b�g�I�[�_�[����ꊷ���� */
				bm_msblsb(buf,&buf,BM_PADBITS_MAX);
				if(setx_len!=BM_PADBYTES_MAX){
					buf <<= (BM_PADBITS_MAX-BM_PADBITS(src->bm_pad));
				}
			}
			/* ������on/off��ϊ����� */
			if(dst->bm_onoff != src->bm_onoff){
				buf = ~buf;
			}
			bm_swab(&buf);	/* swap byte��ϊ����� */
			memcpy(dst->image+setpos, (char*)&buf,setx_len);	/* �������� */
		}
	}
}

/*
**********************************************************************
*	bm_read:	�C���[�W����r�b�g���ǂݏo��
**********************************************************************
*/
void
bm_read(
struct	bm_image_s *dst,		/* �r�b�g�}�b�v�C���[�W */
bm_maxpad_t		   *pattern)	/* �r�b�g�� */
{
	int				read_x,read_y;	/* �ǂݍ��݈ʒu */
	int				len,res_len;	/* �P��̓ǂݍ��݃r�b�g�� */
	bm_maxpad_t		buf;
	char			*p;				/* �ǂݍ��݃r�b�g��̃A�h���X(byte) */
	int				nx,xloop;		/* x�̓ǂݍ��݉� */
	int				ny;				/* y�̓ǂݍ��݉� */

/* -----	procedure */

	/* �ǂݍ��݈ʒu������������ */
	read_x = dst->x;
	read_y = dst->y;

	p 		= (char *)pattern;
	len		= dst->bits;			/* �p�f�B���O�̃r�b�g�� */
	xloop	= dst->w/len;
	res_len = dst->w%len;			/* �p�f�B���O�P�ʂ̒[���� */

	/* �����̉񐔓ǂݍ��� */
	for(ny=0;ny<dst->h;ny++){
		/* �p�f�B���O�T�C�Y�����ǂݍ��� */
		for(nx=0;nx<xloop;nx++){
			bm_getpos(dst,read_x,read_y,&buf,len);
			memcpy(p, (char*)&buf,dst->bytes);
			read_x += dst->bits;
			p += dst->bytes;
		}
		if(res_len!=0){
			/* x�����̒[������ǂݍ��� */
			bm_getpos(dst,read_x,read_y,&buf,res_len);
			memcpy(p, (char*)&buf,dst->bytes);
			read_x += dst->bits;
			p += dst->bytes;
		}
		/* �������ɓǂݍ��݈ʒu���C������ */
		read_x = dst->x;
		read_y ++;
		if(read_y > dst->ih){
			break;
		}
	}
}

/*
**********************************************************************
*	bm_write:	�r�b�g����C���[�W�ɏ�������
**********************************************************************
*/
void
bm_write(
struct	bm_image_s *dst,		/* �r�b�g�}�b�v�C���[�W */
bm_maxpad_t		   *pattern)	/* �r�b�g�� */
{
	int				write_x,write_y;/* �������݈ʒu */
	int				len,res_len;	/* �P��̏������݃r�b�g�� */
	char			*p;				/* �������݃r�b�g���byte�A�h���X */
	bm_maxpad_t		buf;			/* �������݃o�b�t�@ */
	int				nx,xloop;		/* x�̏������݉� */
	int				ny;				/* y�̏������݉� */

/* -----	procedure */

	/* �������݈ʒu������������ */
	write_x = dst->x;
	write_y = dst->y;

	p 		= (char *)pattern;
	len		= dst->bits;			/* �p�f�B���O�̃r�b�g�� */
	xloop	= dst->w/len;
	res_len = dst->w%len;			/* �p�f�B���O�P�ʂ̒[���� */
	buf 	= 0;

	/* �����̉񐔏������� */
	for(ny=0;ny<dst->h;ny++){
		/* �p�f�B���O�T�C�Y������������ */
		for(nx=0;nx<xloop;nx++){
			memcpy((char*)&buf,p,dst->bytes);
			bm_setpos(dst,write_x,write_y,buf,len);
			write_x += dst->bits;
			p += dst->bytes;
		}
		if(res_len!=0){
			/* x�����̒[�������������� */
			memcpy((char*)&buf,p,dst->bytes);
			bm_setpos(dst,write_x,write_y,buf,res_len);
			write_x += dst->bits;
			p += dst->bytes;
		}
		/* �������ɏ������݈ʒu���C������ */
		write_x = dst->x;
		write_y ++;
		if(write_y > dst->ih){
			break;
		}
	}
}

/*
**********************************************************************
*	bm_kill:	�r�b�g�C���[�W�̃G���A���������
**********************************************************************
*/
void 
bm_kill(
struct bm_image_s *bm)		/* bit-map�C���[�W */
{
/* -----	procedure */

	if(bm!=NULL){
		if(bm->header!=NULL)	free(bm->header);
		if(bm->image!=NULL)		free(bm->image);
		free(bm);
	}
}
