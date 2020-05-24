#include <stdlib.h>
#include <stdint.h>

#include "c.h"

#include "algebra.h"
#include "image.h"

struct {
	Lattice lcId;
	Lattice lc2Id00;
	Lattice lc2Id10;
	Lattice lc2Id01;
	Lattice lc2Id11;
	Lattice lcQQ00;
	Lattice lcQQ10;
	int set;
} lattices = { .set = 0 };

void
setlattices() {
	lattices.lcId = lattice((Point){ .x = 1, .y = 0 }, (Point){ .x = 0, .y = 1 }, (Point){ .x = 0, .y = 0 });
	lattices.lc2Id00 = lattice((Point){ .x = 2, .y = 0 }, (Point){ .x = 0, .y = 2 }, (Point){ .x = 0, .y = 0 });
	lattices.lc2Id10 = lattice((Point){ .x = 2, .y = 0 }, (Point){ .x = 0, .y = 2 }, (Point){ .x = 1, .y = 0 });
	lattices.lc2Id01 = lattice((Point){ .x = 2, .y = 0 }, (Point){ .x = 0, .y = 2 }, (Point){ .x = 0, .y = 1 });
	lattices.lc2Id11 = lattice((Point){ .x = 2, .y = 0 }, (Point){ .x = 0, .y = 2 }, (Point){ .x = 1, .y = 1 });
	lattices.lcQQ00 = lattice((Point){ .x = 1, .y = -1 }, (Point){ .x = 1, .y = 1 }, (Point){ .x = 0, .y = 0 });
	lattices.lcQQ10 = lattice((Point){ .x = 1, .y = -1 }, (Point){ .x = 1, .y = 1 }, (Point){ .x = 1, .y = 0 });
	lattices.set = 1;
}

Channel *
newchan(ChannelType typ, ChannelDataType dtyp, Lattice lc)
{
	Channel *ch;

	if ((ch = calloc(1, sizeof(Channel))) == nil)
		return nil;

	ch->typ = typ;
	ch->dtyp = dtyp;
	ch->lc = lc;
	ch->data = nil;

	return ch;
}

int
initchan(Channel *ch, Rect r)
{
	long len, bytesize;	

	if (ch->data != nil)
		return -1;
	
	ch->r = r;
	bytesize = chanbytesize(ch->dtyp);
	len = dRx(ch->r) * dRy(ch->r);

	if ((ch->data = calloc(len, bytesize)) == nil) {
		return -1;
	}

	return 0;
}

void
freechan(Channel *ch)
{
	if(ch->data != nil) {
		free(ch->data);
	}
	free(ch);
}

Image *
newim(CompositeType typ)
{
	Image *im = nil;

	Channel **ch;
	ChannelDataType dtyp;
	ChannelType ctyp[3] = { ICred, ICgreen, ICblue };
	Lattice lc[3];

	int i;	

	if ((im = (Image *)calloc(1, sizeof(Image))) == nil)
		goto newim_error;

	if (lattices.set == 0) setlattices();

	im->typ = typ;
	switch (im->typ) {
		case Imono_uint8:
			dtyp = IDuint8;
			im->nchans = 1;
			ctyp[0] = ICmono;
			lc[0] = lattices.lcId;
			break;

		case Imono_uint16:
			dtyp = IDuint16;
			im->nchans = 1;
			ctyp[0] = ICmono;
			lc[0] = lattices.lcId;
			break;

		case Irgb_uint8:
			dtyp = IDuint8;
			im->nchans = 3;
			lc[0] = lc[1] = lc[2] = lattices.lcId;			
			break;

		case Irgb_uint16:
			dtyp = IDuint16;
			im->nchans = 3;
			lc[0] = lc[1] = lc[2] = lattices.lcId;			
			break;			

		case Irgb_bayer_rggb_uint8:
			dtyp = IDuint8;
			im->nchans = 3;
			lc[0] = lattices.lc2Id00;
			lc[1] = lattices.lcQQ10;
			lc[2] = lattices.lc2Id11;			
			break;

		case Irgb_bayer_rggb_uint16:
			dtyp = IDuint16;
			im->nchans = 3;
			lc[0] = lattices.lc2Id00;
			lc[1] = lattices.lcQQ10;
			lc[2] = lattices.lc2Id11;			
			break;

		default:
			im->nchans = 0;
			break;
	}

	if ((im->chans = calloc(im->nchans, sizeof(Channel *))) == nil)
		goto newim_error;
	
	ch = im->chans;
	for (i = 0; i < im->nchans; i++, ch++) {
		if ((*ch = newchan(ctyp[i], dtyp, lc[i])) == nil)
			goto newim_error;
	}

	return im;
newim_error:
	if (im != nil) freeim(im);
	return nil;
}

void
freeim(Image *im)
{
	int i;
	Channel **ch = im->chans;

	if(im->chans != nil) {
		for (i = 0; i < im->nchans; i++, ch++)
			freechan(*ch);

		free(im->chans);
	}
	free(im);
}

void *
pixelptr(Image *im, int chan, Point pt)
{
	Channel *ch;
	long idx;

	ch = im->chans[chan];

	if (!PinR(pt, ch->r))
		return nil;

	if (!PinLc(pt, ch->lc))
		return nil;

	idx = dPy((ch->r).min, pt) * dRx(ch->r) + dPx((ch->r).min, pt);

	switch (ch->dtyp) {
		case IDuint8:
			return (uint8_t *)(ch->data) + idx;
		case IDuint16:
			return (uint16_t *)(ch->data) + idx;
		default:
			return nil;
	}
}

/* return values:
	0x0a big endian
	0x0b PDP-11
	0x0c Honeywell 316
	0x0d little endian
*/
uint8_t
endianness()
{
	union {
		uint32_t i32;
		uint8_t i8[4];
	} i = { .i32 = 0x0a0b0c0d };

	return i.i8[4];
}