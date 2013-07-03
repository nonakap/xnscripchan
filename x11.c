/*	$Id: x11.c,v 1.13 2002/01/18 18:23:39 nonaka Exp $	*/

#ifndef	FONTSET
#define	FONTSET	"-ricoh-gothic-%s-r-normal--%ld-*-*-*-*-*-*-*"
#endif

/*
 * Copyright (c) 2001, 2002 NONAKA Kimihiro <aw9k-nnk@asahi-net.or.jp>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "nscr.h"
#include "xdimage.h"

Display *display;
int screen;
Window window;
Window root;
GC gc;
Visual *visual;
int depth;
Colormap cmap;
int black, white;

static unsigned int rmask, gmask, bmask;
static int rshift = 0, gshift = 0, bshift = 0;
static int rmasklen = 0, gmasklen = 0, bmasklen = 0;

#define	XCOLOR2PIXEL(xc,pixel)						\
do {									\
	(pixel) = ((((xc).red / (256/(1<<rmasklen))) << rshift) & rmask) \
	    | ((((xc).green / (256/(1<<gmasklen))) << gshift) & gmask)	\
	    | ((((xc).blue / (256/(1<<bmasklen))) << bshift) & bmask);	\
} while (/*CONSTCOND*/ 0)

static Atom proto, delwin;
static XFontSet fontset;

static point_t pos_last_click;

#define	EVENT_MASK	(Button1MotionMask	\
			 | Button2MotionMask	\
			 | Button3MotionMask	\
			 | PointerMotionMask	\
			 | KeyPressMask		\
			 | KeyReleaseMask	\
			 | ButtonPressMask	\
			 | ButtonReleaseMask	\
			 | EnterWindowMask	\
			 | LeaveWindowMask	\
			 | ExposureMask)

static int process_events(void);

int
gui_init(int width, int height)
{
	XSizeHints size;
	unsigned int i;

	display = XOpenDisplay(NULL);
	if (display == NULL) {
		ERRORMSG(("Can't open display.\n"));
		return 1;
	}

	root = DefaultRootWindow(display);
	screen = DefaultScreen(display);
	visual = DefaultVisual(display, screen);
	depth = DefaultDepth(display, screen);
	cmap = XDefaultColormap(display, screen);
	black = BlackPixel(display, screen);
	white = WhitePixel(display, screen);

	if (visual->class != TrueColor && visual->class != DirectColor) {
		ERRORMSG(("No support visual class\n"));
		XCloseDisplay(display);
		return 1;
	}

	window = XCreateSimpleWindow(display, root, 0, 0, width, height,
				     0, white, black);
	if (window == 0) {
		ERRORMSG(("Can't create window.\n"));
		XCloseDisplay(display);
		return 1;
	}

	gc = XCreateGC(display, root, 0, NULL);
	XSetForeground(display, gc, white);
	XSetBackground(display, gc, black);

	/* Calc color mask len */
	for (rmask = i = visual->red_mask; (i & 1) == 0; i >>= 1)
		rshift++;
	for (; i & 1; i >>= 1)
		rmasklen++;
	for (gmask = i = visual->green_mask; (i & 1) == 0; i >>= 1)
		gshift++;
	for (; i & 1; i >>= 1)
		gmasklen++;
	for (bmask = i = visual->blue_mask; (i & 1) == 0; i >>= 1)
		bshift++;
	for (; i & 1; i >>= 1)
		bmasklen++;

	XStoreName(display, window, "Xnscripchan");

	/* Set window size */
	size.flags = PMinSize | PMaxSize;
	size.min_width = width;
	size.min_height = height;
	size.max_width = width;
	size.max_height = height;
	XSetNormalHints(display, window, &size);

	proto = XInternAtom(display, "WM_PROTOCOLS", 0);
	delwin = XInternAtom(display, "WM_DELETE_WINDOW", 0);
	XSetWMProtocols(display, window, &delwin, 1);

	XSelectInput(display, window, EVENT_MASK);

	XMapRaised(display, window);
	XSync(display, False);

	return 0;
}

void
gui_close(void)
{

	XCloseDisplay(display);
}

int
engine_exec(void)
{

	for (;;) {
		if (XPending(display) > 0) {
			if (process_events())
				break;
		} else if ((state == STATE_BUTTONWAIT)
		    || (state == STATE_SELECTWAIT)
		    || (state == STATE_CLICKPAGEWAIT)
		    || (state == STATE_CLICKWAIT)) {
			if (XPending(display) > 0) {
				if (process_events())
					break;
			}
		} else if (state == STATE_WAITTIMER) {
			if (XPending(display) > 0) {
				if (process_events())
					break;
			}
			if (waittimer < timer_get()) {
				waittimer = 0;
				state = STATE_COMMAND;
			}
		} else {
			if (parse())
				break;
		}
	}

	return 0;
}

static int
process_events(void)
{
	XEvent ev;
	point_t point;

	XNextEvent(display, &ev);
	switch (ev.type) {
	case ClientMessage:
		if ((ev.xclient.message_type == proto) &&
		    ((Atom)ev.xclient.data.l[0] == delwin))
			return 1;
		break;

	case Expose:
		if (ev.xexpose.count == 0) {
			rect_t rect;
			rect.left = rect.top = 0;
			rect.width = SCREEN_WIDTH;
			rect.height = SCREEN_HEIGHT;
			redraw(&rect);
		}
		break;

	case ButtonPress:
		pos_last_click.x = ev.xbutton.x;
		pos_last_click.y = ev.xbutton.y;

		switch (ev.xbutton.button) {
		case Button1:
			if (trapon)
				is_trap = 1;

			switch (state) {
			case STATE_BUTTONWAIT:
				button_do(0, &pos_last_click);
				break;

			case STATE_SELECTWAIT:
				select_do(0, &pos_last_click);
				break;

			case STATE_CLICKWAIT:
				state = STATE_COMMAND;
				break;

			case STATE_CLICKPAGEWAIT:
				newpage();
				state = STATE_COMMAND;
				break;

			case STATE_WAITTIMER:
				if (cancel_timer)
					waittimer = 0;
			}
			break;

		case Button3:
			switch (state) {
			case STATE_BUTTONWAIT:
				button_do(1, &pos_last_click);
				break;
			}
		}
		break;

	case MotionNotify:
		point.x = ev.xmotion.x;
		point.y = ev.xmotion.y;

		switch (state) {
		case STATE_BUTTONWAIT:
			button_move(&point);
			break;

		case STATE_SELECTWAIT:
			select_move(&point);
			break;
		}
		break;
	}
	return 0;
}

void
gui_caption(unsigned char *str)
{
	unsigned char title[256];

	strlcpy(title, "Xnscripchan - ", sizeof(title));
	strlcat(title, sjis2euc_str(str), sizeof(title));
	title[sizeof(title) - 1] = '\0';

	XStoreName(display, window, title);
}

void
font_init(long fontx, long fonty, long bold)
{
	char buf[1024];
	char **missing_list;
	int missing_num;

	UNUSED(fonty);

	if (fontset != NULL) {
		XFreeFontSet(display, fontset);
		fontset = NULL;
	}

	snprintf(buf, sizeof(buf), FONTSET, bold ? "bold" : "meduim", fontx);
	fontset = XCreateFontSet(display, buf, &missing_list, &missing_num,
	    NULL);
	_ASSERT(fontset != NULL);
}

void
sys_draw_char(int x, int y, unsigned short c, long color)
{
	char tmp[3] = {0, 0, 0};

	tmp[0] = c >> 8;
	tmp[1] = c & 0xff;

	if (CFONT->is_shadow) {
		XSetForeground(display, gc, black);
		XmbDrawString(display,window,fontset,gc, x - 1, y + 14, tmp, 2);
		XmbDrawString(display,window,fontset,gc, x    , y + 13, tmp, 2);
		XmbDrawString(display,window,fontset,gc, x + 1, y + 14, tmp, 2);
		XmbDrawString(display,window,fontset,gc, x    , y + 15, tmp, 2);
	}

	XSetForeground(display, gc, color);
	XmbDrawString(display, window, fontset, gc, x    , y + 14, tmp, 2);
}

void
sys_copy_area(image_t *img, int left, int top, int width, int height, int src_left, int src_top)
{
	xdimage_t *current;

	current = img_create_xdimage_from_image(display, img);
	img_create_pixmap_from_xdimage(current, window);

	XCopyArea(display, current->pixmap, window, gc,
	    left, top, width, height, src_left, src_top);
	XSync(display, False);

	img_destroy_xdimage(current);
}

void
sys_alloc_color(long color, int *valp)
{
	XColor xc;

	xc.red = (color >> 16) & 0xff;
	xc.green = (color >> 8) & 0xff;
	xc.blue = color & 0xff;
	XCOLOR2PIXEL(xc, *valp);
}

void
sys_mouse_getpos(point_t *p)
{
	Window root_window, child;
	int winx, winy;
	int rootx, rooty;
	unsigned int mask;

	XQueryPointer(display, window, &root_window, &child, &rootx, &rooty,
	    &winx, &winy, &mask);

	p->x = winx;
	p->y = winy;
}

void
mouse_getpos_lastclick(point_t *p)
{

	*p = pos_last_click;
}
