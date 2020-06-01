#ifndef __CASTRO_X11EVENT_H__
#define __CASTRO_X11EVENT_H__

#include <stdint.h>
#include <stdlib.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "../3rdparty/pt/pt.h"

#include "../c.h"

#include "../algebra.h"
#include "../colour.h"
#include "../image.h"

#include "draw.h"
#include "wevent.h"


WinEvent unpackwin(Wevents *we, xcb_generic_event_t *ev);
MouseEvent unpackmouse(Wevents *we, xcb_generic_event_t *ev);
KbdEvent unpackkbd(Wevents *we, xcb_generic_event_t *ev);

Win *wid2win(Wevents *we, uint32_t wid);

Wevents *
newevs(Connection *c)
{
	Wevents *we;

	if ((we = calloc(1, sizeof(Wevents))) == nil)
		return nil;

	we->c = c;

	we->wq = (WinEventQueue) pt_queue_init();
	we->mq = (MouseEventQueue) pt_queue_init();
	we->kq = (KbdEventQueue) pt_queue_init();

	we->wlist = nil;

	return we;
}

void
freeevs(Wevents *we)
{
	llnode *node = we->wlist, *next;

	while(node) {
		next = node->next;
		free(node);
		node = next;
	}

	free(we);
}

int
regevs(Wevents *we, Win *w, uint32_t mask)
{
	llnode *node;
	xcb_void_cookie_t cookie;

	if (!wid2win(we, w->id)) {
		if ((node = malloc(sizeof(llnode))) == nil)
			return -1;

		node->next = we->wlist;
		node->data = (void *)w;
		we->wlist = node;
	}

	/* XXX this will be wrong if something goes wrong in the request */
	we->mask |= mask;

	cookie = xcb_change_window_attributes_checked(we->c, w->id, XCB_CW_EVENT_MASK, &(we->mask));

	if (xcb_request_check(we->c, cookie))
		return -2;

	return 0;
}

int 
unregevs(Wevents *we, Win *w, uint32_t mask)
{
	xcb_void_cookie_t cookie;

	/* XXX this will be wrong if something goes wrong in the request */
	we->mask &= ~mask;

	cookie = xcb_change_window_attributes_checked(we->c, w->id, XCB_CW_EVENT_MASK, &(we->mask));

	if (xcb_request_check(we->c, cookie))
		return -2;

	return 0;
}

void
fetchevs(struct pt *pt, Wevents *we)
{
	xcb_generic_event_t *ev;

	pt_begin(pt);

	for (;;) {
		pt_wait(pt, ev = xcb_poll_for_event(we->c));

		if (ev->response_type & 0x7f & X_MOUSE_EVENT_RESPONSE)
			pt_queue_push(&we->mq, unpackmouse(we, ev));

		if (ev->response_type & 0x7f & X_MOUSE_EVENT_RESPONSE)
			pt_queue_push(&we->kq, unpackkbd(we, ev));

		if (ev->response_type & 0x7f & X_WIN_EVENT_RESPONSE)
			pt_queue_push(&we->wq, unpackwin(we, ev));

		if ((ev->response_type & 0x7f) == 0) /* error */
			pt_queue_push(&we->wq, unpackwin(we, ev));

		free(ev);
	}

	pt_end(pt);	
}

WinEvent
unpackwin(Wevents *we, xcb_generic_event_t *ev)
{
	WinEvent e = {0};
	uint32_t wid;

	e.t = XCB_TIME_CURRENT_TIME;
	e.typ = ev->response_type & 0x7f;
	switch (e.typ) {
		case WEerror: {
			xcb_generic_error_t *xev = (xcb_generic_error_t *)ev;
			wid = 0;
			e.data.error.code = xev->error_code;
			e.data.error.opmajor = xev->major_code;
			e.data.error.opminor = xev->minor_code;
			break;
		}
		case WEexpose: {
			xcb_expose_event_t *xev = (xcb_expose_event_t *)ev;
			wid = xev->window;
			e.data.r = (Rect){
				 .min = { .x = xev->x, .y = xev->y},
				 .max = { .x = xev->width, .y = xev->height}};
			e.data.r.max.x += e.data.r.min.x;
			e.data.r.max.y += e.data.r.min.y;
			break;
		}
		case WEmove: {
			xcb_configure_notify_event_t *xev = (xcb_configure_notify_event_t *)ev;
			wid = xev->window;
			e.data.r = (Rect){
				 .min = { .x = xev->x, .y = xev->y},
				 .max = { .x = xev->width, .y = xev->height}};
			e.data.r.max.x += e.data.r.min.x;
			e.data.r.max.y += e.data.r.min.y;
			break;
		}
		case WEshow: {
			xcb_map_notify_event_t *xev = (xcb_map_notify_event_t *)ev;
			wid = xev->window;
			e.data.visible = 1;
			break;
		}
		case WEhide: {
			xcb_unmap_notify_event_t *xev = (xcb_unmap_notify_event_t *)ev;
			wid = xev->window;
			e.data.visible = 0;
			break;
		}
		default:
			break;
	}

	e.w = wid2win(we, wid);

	return e;
}

MouseEvent
unpackmouse(Wevents *we, xcb_generic_event_t *ev)
{
	MouseEvent e = {0};
	uint32_t wid;

	e.typ = ev->response_type & 0x7f;
	switch (e.typ) {
		case MEpress:
		case MErelease: {
			xcb_button_press_event_t *xev = (xcb_button_press_event_t *)ev;
			wid = xev->event;
			e.t = xev->time;
			e.p = (Point){ .x = xev->event_x, .y = xev->event_y };
			e.button = 0x01 << (xev->detail);
			e.modifiers = ((xev->state & 0xff00) >> 8) | ((xev->state & 0x00ff) << 8);
			break;
		}
		case MEmotion: {
			xcb_motion_notify_event_t *xev = (xcb_motion_notify_event_t *)ev;
			wid = xev->event;
			e.t = xev->time;
			e.p = (Point){ .x = xev->event_x, .y = xev->event_y };
			e.button = 0;
			e.modifiers = ((xev->state & 0xff00) >> 8) | ((xev->state & 0x00ff) << 8);
			break;
		}
		case MEenter:
		case MEleave: {
			xcb_enter_notify_event_t *xev = (xcb_enter_notify_event_t *)ev;
			wid = xev->event;
			e.t = xev->time;
			e.p = (Point){ .x = xev->event_x, .y = xev->event_y };
			e.button = 0;
			e.modifiers = ((xev->state & 0xff00) >> 8) | ((xev->state & 0x00ff) << 8);
			break;
		}
		default:
			break;
	}

	e.w = wid2win(we, wid);

	return e;
}

KbdEvent
unpackkbd(Wevents *we, xcb_generic_event_t *ev)
{
	KbdEvent e = {0};
	uint32_t wid;

	e.typ = ev->response_type & 0x7f;
	switch (e.typ) {
		case KEpress:
		case KErelease: {
			xcb_key_press_event_t *xev = (xcb_key_press_event_t *)ev;
			wid = xev->event;
			e.t = xev->time;
			e.keycode = xev->detail;
			e.modifiers = ((xev->state & 0xff00) >> 8) | ((xev->state & 0x00ff) << 8);
			break;
		}
		default:
			break;
	}

	e.w = wid2win(we, wid);

	return e;
}

Win *
wid2win(Wevents *we, uint32_t wid)
{
	llnode *node = we->wlist;

	while (node) {
		if ((Win *)(node->data))
			if (((Win *)(node->data))->id == wid) return (Win *)(node->data);
		node = node->next;
	}

	return nil;
}

#endif