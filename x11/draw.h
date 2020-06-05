/* requires: xcb.h, xcb/xproto.h, stdint.h, ../3rdparty/pt/pt.h algebra.h, colour.h, image.h */

#ifndef __CASTRO_X11_DRAW_H__
#define __CASTRO_X11_DRAW_H__

#define BORDER_WIDTH 0


typedef xcb_connection_t Connection;

Connection *newconn(char *connstr);
void freeconn(Connection *c);

int flushconn(Connection *c);


typedef struct Display Display;
struct Display {
	Connection *c;
	xcb_screen_t *s;
	struct Win *root;
	xcb_visualid_t vid;
	xcb_visualtype_t *v;
	xcb_gcontext_t gc0id;
	SCProfile *cp;
};

Display *newdisplay(Connection *c, int n, SCProfile *cp);
void freedisplay(Display *d);


typedef struct Win Win;
struct Win {
	Display *disp;
	Win *parent;
	uint32_t id;
	Rect r;
	int visible;
};

Win *newwin(Win *parent, Rect r);
void freewin(Win *w);

Win rootwin(Display *disp);

int showwin(Win *w);
int hidewin(Win *w);
int movewin(Win *w, Rect r);

void updatewin(Win *w);

typedef struct Raster Raster;
struct Raster {
	Display *disp;
	uint32_t id;
	Rect r;
};

Raster *newraster(Display *disp, Rect r);
void freeraster(Raster *rst);

int ldraster(Raster *rst, Image *im, Rect r_from, Rect r_to);

int drawraster(Win *w, Raster *rst, Rect r_from, Rect r_to);

#endif



