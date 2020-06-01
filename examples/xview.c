#include <stdio.h>
#include <stdint.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "../3rdparty/pt/pt.h"

#include "../c.h"
#include "../sys.h"

#include "../algebra.h"
#include "../colour.h"
#include "../image.h"
#include "../io/raw.h"

#include "../x11/draw.h"
#include "../x11/wevent.h"


struct Ctxt {
	Connection *c;
	Display *disp;
	Win *w;
	Wevents *evs;
	Image *im;
	Raster *rst;
};

void cleanup(struct Ctxt *ctxt);

int draw(Win *w, Raster *rst, Rect r);

void recvwin(struct pt *pt, struct Ctxt *ctxt);
void recvkbd(struct pt *pt, struct Ctxt *ctxt);
void recvmouse(struct pt *pt, struct Ctxt *ctxt);

struct pt pt_wevent = pt_init();
struct pt pt_recvw = pt_init();
struct pt pt_recvk = pt_init();
struct pt pt_recvm = pt_init();

int
main(int argc, char *argv[])
{
	char *fn = (argc > 1) ? argv[1] : "/dev/stdin", *errmsg = nil;

	struct Ctxt ctxt = { 0 };

	Rect r;

	if (!(ctxt.c = newconn(nil))) {
		errmsg = "view: cannot open X11 connection\n";
		goto main_cleanup;
	}

	if (!(ctxt.disp = newdisplay(ctxt.c, 0, nil))) {
		errmsg = "view: cannot get X11 display\n";
		goto main_cleanup;
	}

	if (!(ctxt.im = rdim(fn))) {
		errmsg = "view: cannot load image\n";
		goto main_cleanup;
	}
	
	r = bbox(ctxt.im);

	if (!(ctxt.rst = newraster(ctxt.disp, r))) {
		errmsg = "view: cannot allocate raster\n";
		goto main_cleanup;
	}

	if (!ldraster(ctxt.rst, ctxt.im, r, r)) {
		errmsg = "view: cannot copy image to X11 raster\n";
		goto main_cleanup;
	}

	if (!(ctxt.w = newwin(ctxt.disp->root, r))) {
		errmsg = "view: cannot create X11 window\n";
		goto main_cleanup;
	}

	if (!(ctxt.evs = newevs(ctxt.c))) {
		errmsg = "view: cannot create event stream\n";
		goto main_cleanup;
	}

	if (!regevs(ctxt.evs, ctxt.w, EMwin | EMmouse | EMkbd)) {
		errmsg = "view: cannot register for events\n";
		goto main_cleanup;
	}

	showwin(ctxt.w);

	draw(ctxt.w, ctxt.rst, r);

	while (pt_status(&pt_wevent) == 0 && pt_status(&pt_recvm) == 0 && pt_status(&pt_recvk) == 0 && pt_status(&pt_recvw) == 0) {
		fetchevs(&pt_wevent, ctxt.evs);

		recvmouse(&pt_recvm, &ctxt);
		recvkbd(&pt_recvk, &ctxt);
		recvwin(&pt_recvw, &ctxt);
	}

main_cleanup:
	cleanup(&ctxt);

	if (errmsg) {
		wterror(errmsg);
		return -1;
	}

	return 0;
}

void
cleanup(struct Ctxt *ctxt)
{
	if (ctxt->rst) freeraster(ctxt->rst);
	if (ctxt->im) freeim(ctxt->im);
	if (ctxt->evs) freeevs(ctxt->evs);
	if (ctxt->w) freewin(ctxt->w);
	if (ctxt->disp) freedisplay(ctxt->disp);
	if (ctxt->c) freeconn(ctxt->c);	
}

void
recvwin(struct pt *pt, struct Ctxt *ctxt)
{
	WinEvent *e;

	pt_begin(pt);

	for (;;) {
		pt_wait(pt, !pt_queue_empty(&ctxt->evs->wq));

		e = pt_queue_pop(&ctxt->evs->wq);

		/* XXX */
		if (e->typ == WEerror)
			pt_exit(pt, 1);
		if (draw(ctxt->w, ctxt->rst, ctxt->w->r))
			pt_exit(pt, 2);
	}

	pt_end(pt);
}

void
recvmouse(struct pt *pt, struct Ctxt *ctxt)
{
	MouseEvent *e;

	pt_begin(pt);

	for (;;) {
		pt_wait(pt, !pt_queue_empty(&ctxt->evs->mq));

		e = pt_queue_pop(&ctxt->evs->mq);

		/* XXX */
	}

	pt_end(pt);
}

void
recvkbd(struct pt *pt, struct Ctxt *ctxt)
{
	KbdEvent *e;

	pt_begin(pt);

	for (;;) {
		pt_wait(pt, !pt_queue_empty(&ctxt->evs->kq));

		e = pt_queue_pop(&ctxt->evs->kq);

		/* XXX */
	}

	pt_end(pt);
}

int
draw(Win *w, Raster *rst, Rect r)
{
	drawraster(w, rst, r, r);
	return flushwin(w);
}