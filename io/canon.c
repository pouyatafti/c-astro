#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../c.h"

#include "../sys.h"

#include "../algebra.h"
#include "../image.h"
#include "canon.h"

int gettagdata(Craw2IfdTag *tag);

#if 0
Image *
rdcraw2(char *fn)
{
	FILE *f = nil;
	char *errmsg = nil;
	int i;

	Image *im = nil;

	Craw2Header hdr;
	Craw2Ifd ifd = { .tagcount = 0, tags = nil, nextifdoffset = 0 };
	Craw2IfdTag *tag;

	if ((f = fopen(fn, "rb") == nil) {
		errmsg = "rdcraw2: cannot open file\n";
		goto rdcraw2_error;
	}

	if (fread(&hdr, sizeof(Craw2Header), 1, f) < 1) {
		errmsg = "rdcraw2: cannot read from file\n";
		goto rdcraw2_error;
	}

	if ((hdr.byteorder != CR2_HDR_LTENDIAN) || (hdr.tiffmagic != CR2_HDR_TIFFMAGIC) || (hdr.cr2magic != CR2_HDR_CR2MAGIC) || (hdr.cr2version != CR2_HDR_CR2VERSION)) {
		errmsg = "rdcraw2: unsupported file format\n";
		goto rdcraw2_error;
	}

	if (fsetpos(f, hdr.ifdrawoffset)) {
		errmsg = "rdcraw2: cannot read from file\n";
		goto rdcraw2_error;
	}

	if (fread(&(ifd.tagcount), 2, 1, f) < 1) {
		errmsg = "rdcraw2: cannot read from file\n";
		goto rdcraw2_error;
	}

	if ((ifd.tags = malloc(ifd.tagcount*sizeof(Craw2IfdTag))) == nil) {
		errmsg = "rdcraw2: cannot allocate memory\n";
		goto rdcraw2_error;
	}

	if (fread(ifd.tags, sizeof(Craw2IfdTag), ifd.tagcount, f) < 1) {
		errmsg = "rdcraw2: cannot read from file\n";
		goto rdcraw2_error;
	}

	if (fread(&(ifd.nextifdoffset), 4, 1, f) < 1) {
		errmsg = "rdcraw2: cannot read from file\n";
		goto rdcraw2_error;
	}

	tag = ifd.tags;
	for (i = 0; i < ifd.tagcount; i++, tag++) {
		switch (tag->id) {
			case CR2_TAG_COMPRESSION:
				
				break;
			case CR2_TAG_STRIPOFFSET:
				break;
			case CR2_TAG_STRIPBYTECOUNT:
				break;
			case CR2_TAG_CR2SLICE:
				break;
			case CR2_TAG_RAWFORMAT:
				break;
			default:
				continue;
		}
	}

	goto rdcraw2_cleanup;

rdcraw2_error:
	if (errmsg != nil) {
		wterror(errmsg);
	}
	if (im != nil) freeim(im);
	im = nil;

rdcraw2_cleanup:
	if (ifd.tags != nil) free(ifd.tags);
	if(f != nil) fclose(f);

	return im;	
}
#endif
