#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <png.h>

#include "../c.h"

#include "../sys.h"

#include "../algebra.h"
#include "../image.h"

void user_write(png_structp png, png_bytep data, size_t len);
void user_flush(png_structp png);


int 
wtpng(Image *im, int chan, char *fn)
{
	int retval = 0;
	FILE *f = nil;
	Channel *ch;

	char *errmsg = nil;

	png_structp png = nil;
	png_infop info = nil;

	uint8_t *row = nil;

	png_uint_32 wd, ht;
	int i, depthB, depthb, colour;

	ch = im->chans[chan];

	wd = dRx(ch->r);
	ht = dRy(ch->r);
	depthB = chanbytesize(ch->dtyp);
	depthb = depthB * 8;
	colour = PNG_COLOR_TYPE_GRAY;

	wtlog(10, "fn = %s, wd = %d, ht = %d, depthb = %d, colour = %d, ch->data = %p\n", fn, wd, ht, depthb, colour, (void *)(ch->data));

	if ((f = fopen(fn, "wb")) == nil) {
		errmsg = "cannot open file\n";
		goto wtpng_error;
	}

	wtlog(10, "f = %p\n", f);

	if ((png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nil, nil, nil)) == nil) {
		errmsg = "cannot allocate memory\n";
		goto wtpng_error;
	}	

	if ((info = png_create_info_struct(png)) == nil) {
		errmsg = "cannot allocate memory\n";
		goto wtpng_error;
	}

	if (setjmp(png_jmpbuf(png))) {
		errmsg = "error while creating png file\n";
		goto wtpng_error;
	}

/*	png_init_io(png, f); */
	png_set_write_fn(png, f, user_write, user_flush);

	png_set_IHDR(png, info, wd, ht, depthb, colour, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_set_oFFs(png, info, ch->r.min.x, ch->r.min.y, PNG_OFFSET_PIXEL);

	wtlog(10, "writing info...\n");
	png_write_info(png, info);

	if (depthB > 1 && endianness() != 0x0a) /* not big endian */ {
		png_set_swap(png);
	}

	wtlog(10, "writing rows...\n");

	for (i = 0, row = ch->data; i < ht; i++, row += depthB*wd) {
		png_write_row(png, row);
	}
	
	png_write_end(png, info);

	goto wtpng_cleanup;

wtpng_error:
	wterror(errmsg);
	retval = -1;

wtpng_cleanup:
	if (png != nil) {
		if (info != nil)
			png_destroy_write_struct(&png, &info);
		else
			png_destroy_write_struct(&png, nil);
	}
	if (f != nil) fclose(f);

	return retval;
}

void
user_write(png_structp png, png_bytep data, size_t len) {
	FILE *f = png_get_io_ptr(png);

	if (fwrite(data, 1, len, f) < len)
		png_error(png, "cannot write to file");
}

void
user_flush(png_structp png) {
	FILE *f = png_get_io_ptr(png);

	if (fflush(f))
		png_error(png, "cannot write to file");
}
