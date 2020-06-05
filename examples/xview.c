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

int draw(Win *w, Raster *rst);

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

	Rect Rim, Rrst;

	wtlog(1, "opening connection...\n");
	if (!(ctxt.c = newconn(nil))) {
		errmsg = "cannot open X11 connection\n";
		goto main_cleanup;
	}

	wtlog(1, "getting display...\n");
	if (!(ctxt.disp = newdisplay(ctxt.c, 0, nil))) {
		errmsg = "cannot get X11 display\n";
		goto main_cleanup;
	}

	wtlog(1, "loading image...\n");
	if (!(ctxt.im = rdim(fn))) {
		errmsg = "cannot load image\n";
		goto main_cleanup;
	}
	
	Rim = bbox(ctxt.im);
	Rrst = (Rect){ {0, 0}, {dRx(Rim), dRy(Rim)} };

	wtlog(1, "creating raster...\n");
	if (!(ctxt.rst = newraster(ctxt.disp, Rrst))) {
		errmsg = "cannot create raster\n";
		goto main_cleanup;
	}

	wtlog(1, "loading image into raster...\n");
	if (ldraster(ctxt.rst, ctxt.im, Rim, Rrst)) {
		errmsg = "cannot copy image to X11 raster\n";
		goto main_cleanup;
	}

	wtlog(1, "creating window...\n");
	if (!(ctxt.w = newwin(ctxt.disp->root, Rrst))) {
		errmsg = "cannot create X11 window\n";
		goto main_cleanup;
	}

	wtlog(1, "creating event stream...\n");
	if (!(ctxt.evs = newevs(ctxt.c))) {
		errmsg = "cannot create event stream\n";
		goto main_cleanup;
	}

	wtlog(1, "registering events...\n");
	if (regevs(ctxt.evs, ctxt.w, EMwin | EMmouse | EMkbd)) {
		errmsg = "cannot register for events\n";
		goto main_cleanup;
	}

	showwin(ctxt.w);

	draw(ctxt.w, ctxt.rst);

	wtlog(1, "entering event loop...\n");
	do {
		fetchevs(&pt_wevent, ctxt.evs);

		recvmouse(&pt_recvm, &ctxt);
		recvkbd(&pt_recvk, &ctxt);
		recvwin(&pt_recvw, &ctxt);
	} while (pt_status(&pt_wevent) == 0 && pt_status(&pt_recvm) == 0 && pt_status(&pt_recvk) == 0 && pt_status(&pt_recvw) == 0);


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

		switch (e->typ) {
			case WEexpose:
				wtlog(1, "WEexpose %dx%d+%d+%d\n", dRx(e->data.r), dRy(e->data.r), e->data.r.min.x, e->data.r.min.y);
				break;
			case WEmove:
				wtlog(1, "WEmove %dx%d+%d+%d\n", dRx(e->data.r), dRy(e->data.r), e->data.r.min.x, e->data.r.min.y);
				break;
			case WEshow:
				wtlog(1, "WEshow\n");
				break;
			case WEhide:
				wtlog(1, "WEhide\n");
				break;
			case WEerror:
				wtlog(1, "WEerror\n");
				pt_exit(pt, 1);
				break;
			default:
				break;
		}
		if (e->typ == WEerror)
			pt_exit(pt, 1);
		if (draw(ctxt->w, ctxt->rst))
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

		wtlog(1, "t = %d, typ = %d\n", e->t, e->typ);
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

		wtlog(1, "t = %d, typ = %d\n", e->t, e->typ);
		/* XXX */
	}

	pt_end(pt);
}

int
draw(Win *w, Raster *rst)
{
	return drawraster(w, rst, rst->r, (Rect){ {0, 0}, {dRx(w->r), dRy(w->r)} });
}