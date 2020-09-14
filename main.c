#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/joystick.h>
#include <linux/uinput.h>

#define DEVICE_JS0 "/dev/input/js0"
#define DEVICE_UINPUT "/dev/uinput"

void send_key(u_int16_t key, int value, int sync);
void send_mouse(signed int rel_x, signed int rel_y);
int setup();

struct js_event js_ev;
struct uinput_user_dev uinp;
struct input_event inp_ev;

static int js_fd;
static int uinp_fd;

int main() {
    int res = setup();
    if (res < 0)
        return res;

    while (1) {
        if (read(js_fd, &js_ev, sizeof(struct js_event)) < 0) {
            printf("Failed to read joystick event: %s\n", strerror(errno));
            if (errno == EINTR)
                continue;
            break;
        }

        if (js_ev.type == JS_EVENT_BUTTON) {
            printf("Event: time:%d, value:%d, type:%d, number:%d\n", js_ev.time, js_ev.value, js_ev.type, js_ev.number);
            // if (js_ev.value == 1)
                switch (js_ev.number) {
                case 0:
                    // send_key(KEY_A, js_ev.value);
                    send_mouse(5, 5);
                    break;
                case 1:
                    send_key(KEY_B, js_ev.value, 0);
                    break;
                case 2:
                    send_key(KEY_X, js_ev.value, 1);
                    break;
                case 3:
                    send_key(KEY_Y, js_ev.value, 1);
                }
        } else if (js_ev.type == JS_EVENT_AXIS) {
            printf("Event: time:%d, value:%d, type:%d, number:%d\n", js_ev.time, js_ev.value, js_ev.type, js_ev.number);
        }

        usleep(100);
    }
    
    close(js_fd);
    close(uinp_fd);

    return 0;
}

void send_key(u_int16_t key, signed int value, int sync) {
    memset(&inp_ev, 0, sizeof(inp_ev));
    gettimeofday(&inp_ev.time, NULL);
    inp_ev.type = EV_KEY;
    inp_ev.code = key;
    inp_ev.value = value;
    write(uinp_fd, &inp_ev, sizeof(inp_ev));

    if (value == 0 || sync) {
        memset(&inp_ev, 0, sizeof(inp_ev));
        inp_ev.type = EV_SYN;
        inp_ev.code = SYN_REPORT;
        inp_ev.value = 0;
        write(uinp_fd, &inp_ev, sizeof(inp_ev));
    }
}

void send_mouse(signed int rel_x, signed int rel_y) {
    memset(&inp_ev, 0, sizeof(inp_ev));
    gettimeofday(&inp_ev.time, NULL);
    inp_ev.type = EV_REL;
    inp_ev.code = REL_X;
    inp_ev.value = rel_x;
    write(uinp_fd, &inp_ev, sizeof(inp_ev));

    memset(&inp_ev, 0, sizeof(inp_ev));
    gettimeofday(&inp_ev.time, NULL);
    inp_ev.type = EV_REL;
    inp_ev.code = REL_Y;
    inp_ev.value = rel_y;
    write(uinp_fd, &inp_ev, sizeof(inp_ev));

    memset(&inp_ev, 0, sizeof(inp_ev));
    inp_ev.type = EV_SYN;
    inp_ev.code = SYN_REPORT;
    inp_ev.value = 0;
    write(uinp_fd, &inp_ev, sizeof(inp_ev));
}

int setup() {
    js_fd = open(DEVICE_JS0, O_RDONLY);
    if (js_fd < 0) {
        printf("Failed to open %s\n", DEVICE_JS0);
        return -1;
    }

    uinp_fd = open(DEVICE_UINPUT, O_WRONLY | O_NDELAY);
    if (uinp_fd < 0) {
        printf("Failed to open %s\n", DEVICE_UINPUT);
        return -2;
    }

    memset(&uinp, 0, sizeof(uinp));
    strncpy(uinp.name, "Josefs Controller", UINPUT_MAX_NAME_SIZE);
    uinp.id.version = 4;
    uinp.id.bustype = BUS_VIRTUAL;

    ioctl(uinp_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(uinp_fd, UI_SET_EVBIT, EV_REL);
    ioctl(uinp_fd, UI_SET_RELBIT, REL_X);
    ioctl(uinp_fd, UI_SET_RELBIT, REL_Y);

    for (int i = 0; i < 256; i++) {
        ioctl(uinp_fd, UI_SET_KEYBIT, i);
    }

    // ioctl(uinp_fd, UI_SET_KEYBIT, BTN_MOUSE);
    // ioctl(uinp_fd, UI_SET_KEYBIT, BTN_TOUCH);
    // ioctl(uinp_fd, UI_SET_KEYBIT, BTN_MOUSE);
    ioctl(uinp_fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(uinp_fd, UI_SET_KEYBIT, BTN_MIDDLE);
    ioctl(uinp_fd, UI_SET_KEYBIT, BTN_RIGHT);
    // ioctl(uinp_fd, UI_SET_KEYBIT, BTN_FORWARD);
    // ioctl(uinp_fd, UI_SET_KEYBIT, BTN_BACK);

    write(uinp_fd, &uinp, sizeof(uinp));
    if (ioctl(uinp_fd, UI_DEV_CREATE)) {
        printf("Unable to create UINPUT device.");
        return -1;
    }

    return 0;
}