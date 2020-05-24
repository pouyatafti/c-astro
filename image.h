/* requires: stdint.h, algebra.h */

#ifndef __CASTRO_IMAGE_H__
#define __CASTRO_IMAGE_H__

typedef enum ChannelType ChannelType;
enum ChannelType {
	ICmono,
	ICred,
	ICgreen,
	ICblue
};

typedef enum ChannelDataType ChannelDataType;
enum ChannelDataType {
	IDuint8,
	IDuint16
};
#define chanbytesize(dtyp) ( (dtyp) == IDuint8 ? 1 : 2 )

typedef struct Channel Channel;
struct Channel {
	ChannelType typ;
	ChannelDataType dtyp;
	Lattice lc;
	Rect r;
	void *data;
};

typedef enum CompositeType CompositeType;
enum CompositeType {
	Imono_uint8,
	Imono_uint16,
	Irgb_uint8,
	Irgb_uint16,
	Irgb_bayer_rggb_uint8,
	Irgb_bayer_rggb_uint16
};

typedef struct Image Image;
struct Image {
	CompositeType typ;
	int nchans;
	Channel **chans;
};

Channel *newchan(ChannelType typ, ChannelDataType dtyp, Lattice lc);
int initchan(Channel *ch, Rect r);
void freechan(Channel *ch);

Image *newim(CompositeType typ);
void freeim(Image *im);

/* actual pixel (or nil if non-existent) */
void *pixelptr(Image *im, int chan, Point pt);

/* nearest pixel for RealPoint pt */
#define npixelptr(im, chan, pt) ( pixelptr(im, chan, roundlc(im->chans[chan]->lc, pt)) )
//#define npixelptr(im, chan, pt) ( pixelptr(im, chan, (Point){.x = pt.x, .y = pt.y}) )

uint8_t endianness();

#endif
