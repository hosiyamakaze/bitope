#pragma	comment( user,"@(#)$Workfile: bitope.h $$Revision: 8 $$Date: 25/02/04 19:13 $$NoKeywords: $")
/****************************************************************************
*
*    bitope.h:	�a�h�s�|�l�`�o�I�y���[�V����
*
*
*    ���L�����F
*			bit�P�ʂ̓]���������T�|�[�g���܂��B
*			���m�N���p�^�[��(1�s�N�Z��������̃r�b�g��=1)�̂ݑΉ����܂��B
*			��������ł͍��ɑΉ�����r�b�g��on('1')�Ƃ��܂�
*			bit_map_pad:	�r�b�g�}�b�v�̃p�f�B���O�i�������E�̂݁j
*			bit_map_order:	�o�C�g�̕��я��i���W�n�Ɍ����Ă܂��j
*			bit_byte_order:	�r�b�g�̕��я��iMSB First�̂݁j
*
*****************************************************************************
*/

/* -----	define */
#define		BM_PADBITS_MAX		32				/* �p�f�B���O�̍ő�T�C�Y(bit) */
#define		BM_PADBYTES_MAX		4				/* �p�f�B���O�̍ő�T�C�Y(byte) */
#define		BM_PADBITS(pad)		((int)pad)		/* �p�f�B���O�̃T�C�Y(bit) */
#define		BM_PADBYTES(pad)	((int)pad/8)	/* �p�f�B���O�̃T�C�Y(byte) */
#define		BM_SRC0				((struct bm_image_s *)0)	/* all 0�̃C���[�W */
#define		BM_SRC1				((struct bm_image_s *)1)	/* all 1�̃C���[�W */

/* -----    typedef */
/* �p�f�B���O�̍ő�P�� */
typedef unsigned long int	bm_maxpad_t;

/* bm_pad�̐ݒ�l�i�p�f�B���O�̒P�ʁj */
typedef  enum {
		BM_PAD32	=	32		/* �������E */
}	bm_pad_t;

/* bm_order�̐ݒ�l�i�o�C�g�̕��т��K�肷��j */
typedef enum {
		BM_1ST	=	0x00|0x01,	/* ��P�ی��̍��W�n�I�A���_(0,0)�`(���{)(���{) */
		BM_4TH	=	0x00|0x00	/* ��S�ی��̍��W�n�I�A���_(0.0)�`(���{)(���{) */
		/* 0x10��X��hi-load,0x01��y��hi-load�ł��邱�Ƃ����� */
}	bm_order_t;

/* bm_byte�̐ݒ�l�i�r�b�g�̕��т��K�肷��j */
typedef enum {
		BM_MSBFirst				/* �p�f�B���O�P�ʓ��ł͂l�r�a����Z�b�g���� */
}	bm_byte_t;

/* bm_ftype�̐ݒ�l�ibit-map�t�@�C���̌`���j*/
typedef enum {
		BM_PAINT			/* �y�C���g�u���V(MS_WINDOWS) */
}	bm_ftype_t;

/* bm_onoff�̐ݒ�l(�����r�b�gon���ۂ�) */
typedef enum {
		BM_ON,				/* ����bit��on('1')�ɂ��� */
		BM_OFF				/* ����bit��off('0')�ɂ��� */
}	bm_onoff_t;

/* -----	cnvertoption */
typedef enum {
		BM_NULL =		0x00,	/* �������Ȃ� */	
		BM_X_MIRROR	=	0x10,	/* �������ɋ������]���� */
		BM_Y_MIRROR	=	0x01	/* �������ɋ������]���� */
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
/* �P�C���[�W��\������\���� */
struct bm_image_s {	
	bm_pad_t	bm_pad;		/* �p�f�B���O�̒P�� */
	bm_order_t	bm_order;	/* ���W�n */
	bm_byte_t	bm_byte;	/* �o�C�g����bit�̕��я� */
	bm_onoff_t	bm_onoff;	/* ����on('1')��off('0')�� */
	int			iw;			/* ��`�̈�̕�(bit)�i�p�f�B���O���܂ށj */
	int			ih;			/* ��`�̈�̍���(bit) */
	int			x;			/* �C���[�W�ւ̂��I�t�Z�b�g(bit) */
	int			y;			/* �C���[�W�ւ̂��I�t�Z�b�g(bit) */
	int			w;			/* �C���[�W�̕�(bit)�i�p�f�B���O���܂܂Ȃ��j */
	int			h;			/* �C���[�W�̍���(bit) */
	char		*image;		/* bit�C���[�W */
	int			isize;		/* bit�C���[�W�̃T�C�Y(byte)(iw_byte*ih */
	char		*header;	/* bit-map�t�@�C���̃w�b�_ */
	int			hsize;		/* �w�b�_�̃T�C�Y(byte) */
	int			iw_byte;	/* iw�̃o�C�g�� */
	int			bytes;		/* �p�f�B���O�P�ʂ̃o�C�gBM_PADBYTES(bm_pad)�� */
	int			bits;		/* �p�f�B���O�P�ʂ̃r�b�gBM_BADBITS(bm_pad)�� */
};

/* -----	processor */
void			   bm_swab(		/* swap byte�����s���� */
	bm_maxpad_t* word);	/* �Ώۂ�4�o�C�g*/

struct bm_image_s *bm_create(	/* bit-map�C���[�W���쐬���� */
	int			width,		/* �C���[�W�̕�(bit) */
	int			height,		/* �C���[�W�̍���(bit) */
	bm_pad_t	bm_pad,		/* �p�f�B���O�̒P�� */
	bm_order_t	bm_order,	/* �o�C�g�I�[�_�[���������W�n */
	bm_byte_t	bm_byte,	/* �o�C�g���̃r�b�g�I�[�_�[ */
	bm_onoff_t	bm_onoff,	/* ����on('1')��off('0')�� */
	char		init);		/* �������C���[�W(8-bit�p�^�[��)(MSBFirst) */

struct bm_image_s* bm_load(		/* bit_map�t�@�C�����������ɓǂݍ��� */
	char* bm_file,			/* bit-map�t�@�C���� */
	bm_ftype_t	ftype);		/* bit-map�t�@�C���̌`�� */

int				   bm_save(		/* bit_map�t�@�C���ɕۑ����� */
	struct	bm_image_s* bm,	/* bit-map */
	char* bm_file);		/* bit_map�t�@�C���� */

void			   bm_conv(		/* bit-map�C���[�W��ϊ����� */
	struct bm_image_s* dst,		/* �ϊ����� */
	struct bm_image_s* src,		/* �ϊ����� */
	bm_cnv_t		  mirror);	/* �����ϊ��I�v�V���� */

void			   bm_rop(		/* bit-map�C���[�W��Raster-Operation���� */
	struct	bm_image_s* dst,	/* ���ʐ� */
	struct	bm_image_s* src,	/* ���ʌ�(0/1�̏ꍇ��all 0/1�����肷��) */
	bm_ope_t	rop);		/* Raster-Operation */

char			  *bm_mkheader(	/* bit-map�t�@�C���̃w�b�_�����쐬���� */
	char* curr, /*���݂̃w�b�_*/
	int hsize,	/* �w�b�_�T�C�Y(byte) */
	int	isize,	/* bit-map�C���[�W�̃T�C�Y(byte) */
	int	w,		/* bit-map�̕�(bit) */
	int	h);		/* bit-map�̍���(bit) */

void			   bm_sethead(	/* bit-map�t�@�C���̕ҏW�ς̃w�b�_�����Z�b�g���� */
	struct	bm_image_s* bm,	/* bit-map */
	char* header,			/* �w�b�_�ibit-map�̃t�@�C���`���ɂ���ĈقȂ�j */
	int		size);			/* �w�b�_�̃T�C�Y(byte) */

void			   bm_read(		/* bit-map����r�b�g���ǂݏo�� */
	struct	bm_image_s* dst,/* �r�b�g�}�b�v�C���[�W */
	bm_maxpad_t* pattern);		/* �r�b�g�� */

void			   bm_write(	/* bit-map�Ƀr�b�g����������� */
	struct	bm_image_s* dst,	/* �r�b�g�}�b�v�C���[�W */
	bm_maxpad_t* pattern);			/* �r�b�g�� */

void				   bm_kill(		/* bit-map�C���[�W���J������ */
	struct	bm_image_s* bm);/* bit-map */

#define			   bm_kill0(bm)	{bm_kill((bm));(bm)=NULL;}	/* �����0�ɂ��� */

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

