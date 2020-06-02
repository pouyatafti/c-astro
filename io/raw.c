#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../c.h"

#include "../sys.h"

#include "../algebra.h"
#include "../image.h"

/* ignores endianness -- should work on the same arch */
Image *
rdim(char *fn)
{
	FILE *f = nil;
	Image *im = nil;
	Channel **ch;
	char *errmsg = nil;

	Rect r;
	CompositeType typ;
	int nchans;

	int i;
	long len, bytesize;

	if ((f = fopen(fn, "rb")) == nil) {
		errmsg = "cannot open file\n";
		goto rdim_error;
	}

	if (fread(&typ, sizeof(CompositeType), 1, f) < 1) {
		errmsg = "cannot read from file\n";
		goto rdim_error;
	}

	if (fread(&nchans, sizeof(int), 1, f) < 1) {
		errmsg = "cannot read from file\n";
		goto rdim_error;
	}

	if ((im = newim(typ)) == nil) {
		errmsg = "cannot allocate memory\n";
		goto rdim_error;
	}

	if (im->nchans != nchans) {
		errmsg = "invalid number of channels for type\n";
		goto rdim_error;
	}

	ch = im->chans;
	for (i = 0; i < im->nchans; i++, ch++) {
		if (fread(&r, sizeof(Rect), 1, f) < 1) {
			errmsg = "cannot read from file\n";
			goto rdim_error;
		}		
		if (initchan(*ch, r)) {
			errmsg = "cannot allocate memory\n";
			goto rdim_error;
		}
		len = dRx((*ch)->r) * dRy((*ch)->r);
		bytesize = chanbytesize((*ch)->dtyp);
		if (fread((*ch)->data, bytesize, len, f) < len) {
			errmsg = "cannot read from file\n";
			goto rdim_error;
		}		
	}

	goto rdim_cleanup;

rdim_error:
	wterror(errmsg);
	if (im != nil) freeim(im);
	im = nil;

rdim_cleanup:
	if (f != nil) fclose(f);
	return im;
}

/* ignores endianness -- should work on the same arch */
int
wtim(Image *im, char *fn)
{
	FILE *f = nil;
	Channel **ch;

	int i;
	long len, bytesize;

	if ((f = fopen(fn, "wb")) == nil) {
		wterror("cannot open file\n");
		return -1;
	}

	if (fwrite(&(im->typ), sizeof(CompositeType), 1, f) < 1) {
		wterror("cannot write to file\n");
		fclose(f);
		return -1;
	}

	if (fwrite(&(im->nchans), sizeof(int), 1, f) < 1) {
		wterror("cannot write to file\n");
		fclose(f);
		return -1;
	}

	ch = im->chans;
	for (i = 0; i < im->nchans; i++, ch++) {
		if (fwrite(&((*ch)->r), sizeof(Rect), 1, f) < 1) {
			wterror("cannot write to file\n");
			fclose(f);
			return -1;
		}
		len = dRx((*ch)->r) * dRy((*ch)->r);
		bytesize = chanbytesize((*ch)->dtyp);
		if (fwrite((*ch)->data, bytesize, len, f) < len) {
			wterror("cannot write to file\n");
			fclose(f);
			return -1;
		}		
	}

	fclose(f);
	return 0;
}
