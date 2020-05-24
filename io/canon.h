/* requires: stdint.h, image.h */

#ifndef __CASTRO_CANON_H__
#define __CASTRO_CANON_H__

typefef struct Craw2File Craw2File;
struct Craw2File {
	uint32_t rawaddr;
};

typedef struct Craw2Header Craw2Header;
struct Craw2Header {
	uint16_t byteorder;
	uint16_t tiffmagic;
	uint32_t ifd0offset;
	uint16_t cr2magic;
	uint16_t cr2version;
	uint32_t ifdrawoffset;
};

typedef struct Craw2IfdTag Craw2IfdTag;
struct Craw2IfdTag {
	uint16_t id;
	uint16_t typ;
	uint32_t count;
	uint32_t dataoffset;
};

typedef struct Craw2Ifd Craw2Ifd;
struct Ifd {
	uint16_t tagcount;
	Craw2IfdTag *tags;
	uint32_t nextifdoffset;
};
	
#define CR2_HDR_LTENDIAN 0x4949
#define CR2_HDR_TIFFMAGIC 0x002a
#define CR2_HDR_CR2MAGIC 0x4352
#define CR2_HDR_CR2VERSION 0x200

#define CR2_TAG_COMPRESSION 0x0103
#define CR2_TAG_STRIPOFFSET 0x0111
#define CR2_TAG_STRIPBYTECOUNT 0x0117
#define CR2_TAG_CR2SLICE 0xc640
#define CR2_TAG_RAWFORMAT 0xc6c5

Image *rdcraw2(char *fn);

#endif