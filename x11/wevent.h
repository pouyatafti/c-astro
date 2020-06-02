/* requires: stdint.h xcb.h xcb/xproto.h ../3rdparty/pt/pt.h ../algebra.h draw.h */

#ifndef __CASTRO_X11_WEVENT_H__
#define __CASTRO_X11_WEVENT_H__

#define QUEUE_LEN 32

#define X_WIN_EVENT_MASK ( (uint32_t)(		\
	XCB_EVENT_MASK_EXPOSURE |			\
	XCB_EVENT_MASK_STRUCTURE_NOTIFY) )

#define X_MOUSE_EVENT_MASK ( (uint32_t)(		\
	XCB_EVENT_MASK_BUTTON_PRESS |		\
	XCB_EVENT_MASK_BUTTON_RELEASE |		\
	XCB_EVENT_MASK_BUTTON_MOTION |		\
	XCB_EVENT_MASK_ENTER_WINDOW |		\
	XCB_EVENT_MASK_LEAVE_WINDOW) )

#define X_KBD_EVENT_MASK ( (uint32_t)(		\
	XCB_EVENT_MASK_KEY_PRESS |			\
	XCB_EVENT_MASK_KEY_RELEASE) )


typedef enum WinEventTye WinEventType;
enum WinEventType {
	WEerror = 0,
	WEexpose = XCB_EXPOSE,
	WEmove = XCB_CONFIGURE_NOTIFY,
	WEshow = XCB_MAP_NOTIFY,
	WEhide = XCB_UNMAP_NOTIFY
};

typedef struct WinEvent WinEvent;
struct WinEvent {
	Win *w;
	uint64_t t;		/* clock time in milliseconds */
	uint8_t typ;
	union {
		Rect r;
		int visible;
		struct {
			uint8_t code;
			uint8_t opmajor;
			uint8_t opminor;
		} error;
	} data;
};

typedef pt_queue(WinEvent, QUEUE_LEN) WinEventQueue;

typedef enum MouseEventType MouseEventType;
enum MouseEventType {
	MEpress = XCB_BUTTON_PRESS,
	MErelease = XCB_BUTTON_RELEASE,
	MEmotion = XCB_MOTION_NOTIFY,
	MEenter = XCB_ENTER_NOTIFY,
	MEleave = XCB_LEAVE_NOTIFY
};

typedef struct MouseEvent MouseEvent;
struct MouseEvent {
	Win *w;
	uint64_t t;		/* clock time in milliseconds */
	uint8_t typ;
	Point p;
	uint8_t button;
	uint8_t modifiers;	/* as bits from low to high (mouse buttons first) */
};

typedef pt_queue(MouseEvent, QUEUE_LEN) MouseEventQueue;

typedef enum KbdEventType KbdEventType;
enum KbdEventType {
	KEpress = XCB_KEY_PRESS,
	KErelease = XCB_KEY_RELEASE
};

typedef struct KbdEvent KbdEvent;
struct KbdEvent {
	Win *w;
	uint64_t t;		/* clock time in milliseconds */
	uint8_t typ;
	uint16_t keycode;
	uint8_t modifiers;	/* as bits from low to high (mouse buttons first) */
};

typedef pt_queue(KbdEvent, QUEUE_LEN) KbdEventQueue;

typedef enum WeventsMask WeventsMask;
enum WeventsMask {
	EMwin = X_WIN_EVENT_MASK,
	EMmouse = X_MOUSE_EVENT_MASK,
	EMkbd = X_KBD_EVENT_MASK
};

typedef struct Wevents Wevents;
struct Wevents {
	Connection *c;

	uint32_t mask;

	WinEventQueue wq;
	MouseEventQueue mq;
	KbdEventQueue kq;

	llnode *wlist;
};

Wevents *newevs(Connection *c);
void freeevs(Wevents *we);

int regevs(Wevents *we, Win *w, uint32_t mask);
int unregevs(Wevents *we, Win *w, uint32_t mask);

void fetchevs(struct pt *pt, Wevents *we);


#endif