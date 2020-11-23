#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// #include <X11/Xft/Xft.h>

static int create_overlay(void);
// static void color_create(XftColor *dest, u_int32_t r, u_int32_t g, u_int32_t b, u_int32_t a);
static void *loop_overlay(void *arg);

static XEvent e;
static Display *d;
static Visual *v;
static Window w;
static GC gc;
static int s;

// static XftColor bgc;

static int create_overlay(void) {

    d = XOpenDisplay(NULL);
    if (d == NULL) {
        fprintf(stderr, "Cannot open display\n");
        return 1;
    }

    s = DefaultScreen(d);
    v = DefaultVisual(d, s);

    // XVisualInfo vinfo;
    // XMatchVisualInfo(d, s, 32, TrueColor, &vinfo);

    // color_create(&bgc, 0, 0, 0, 0xaaaa);

    XSetWindowAttributes a;
    // a.background_pixel = 0;
    // a.colormap = XCreateColormap(d, DefaultRootWindow(d), v, AllocNone);
    a.override_redirect = True;
    w = XCreateWindow(d, s,
            50, 50, 300, 300, 1,
            0, InputOutput, v,
            CWOverrideRedirect, &a);
    XSelectInput(d, w, ExposureMask|StructureNotifyMask|KeyPressMask|EnterWindowMask|ButtonPressMask);
    gc = XCreateGC(d, w, 0, 0);

    // Atom wm_delete_window = XInternAtom(d, "WM_DELETE_WINDOW", 0);
    // XSetWMProtocols(d, w, &wm_delete_window, 1);

    // TODO: Only map when used, XUnmapWindow to unmap
    XMapWindow(d, w);
    XRaiseWindow(d, w);

    XClassHint *ch = XAllocClassHint();
    ch->res_name = "sgm";
    ch->res_class = "sgm";
    XSetClassHint(d, w, ch);
    XFree(ch);

    XSetLineAttributes(d, gc, 2, LineSolid, CapButt, JoinBevel);
    XSetFillStyle(d, gc, FillSolid);
    XSync(d, False);

    printf("Created window\n");
    XFlush(d);

    return 0;
}

// static void color_create(XftColor *dest, u_int32_t r, u_int32_t g, u_int32_t b, u_int32_t a) {
//     XRenderColor c = { r, g, b, a };
// 	// XftColorAllocValue(d, v, DefaultColormap(d, s), &c, dest);
// }


// static void draw_text() {

// }

static void *loop_overlay(void *arg) {
    while (1) {
        XNextEvent(d, &e);

        if (e.type == Expose) {
            printf("Draw shit!\n");

            XSetForeground(d, gc, WhitePixel(d, s));
            XFillRectangle(d, w, gc, 10, 10, 20, 20);
        } else if (e.type == ButtonPress) {
            if (e.xbutton.button == 3) {
                XUnmapWindow(d, w);
                XFlush(d);
            }
        } else if (e.type == UnmapNotify) {
            printf("Window is hidden\n");
        } else {
            printf("Event type: %d\n", e.type); 
        }

        usleep(1);
    }
    return NULL;
}

